#include "scheduler/JobConfigLoader.hpp"

#include <cctype>
#include <chrono>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace scheduler {
namespace {

struct JsonValue {
    using Array = std::vector<JsonValue>;
    using Object = std::unordered_map<std::string, JsonValue>;

    std::variant<std::string, std::size_t, Array, Object> value;
};

class JsonParser {
public:
    explicit JsonParser(std::string input)
        : input_(std::move(input)) {}

    JsonValue parse() {
        JsonValue value = parseValue();
        skipWhitespace();
        if (!isAtEnd()) {
            throw error("Unexpected characters after JSON value");
        }
        return value;
    }

private:
    JsonValue parseValue() {
        skipWhitespace();
        if (isAtEnd()) {
            throw error("Unexpected end of input");
        }

        if (peek() == '{') {
            return JsonValue{parseObject()};
        }

        if (peek() == '[') {
            return JsonValue{parseArray()};
        }

        if (peek() == '"') {
            return JsonValue{parseString()};
        }

        if (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            return JsonValue{parseNumber()};
        }

        throw error("Expected an object, array, string, or number");
    }

    JsonValue::Object parseObject() {
        consume('{');

        JsonValue::Object object;
        skipWhitespace();
        if (tryConsume('}')) {
            return object;
        }

        while (true) {
            skipWhitespace();
            if (peek() != '"') {
                throw error("Expected object key");
            }

            std::string key = parseString();
            skipWhitespace();
            consume(':');

            auto [_, inserted] = object.emplace(std::move(key), parseValue());
            if (!inserted) {
                throw error("Duplicate object key");
            }

            skipWhitespace();
            if (tryConsume('}')) {
                return object;
            }
            consume(',');
        }
    }

    JsonValue::Array parseArray() {
        consume('[');

        JsonValue::Array array;
        skipWhitespace();
        if (tryConsume(']')) {
            return array;
        }

        while (true) {
            array.push_back(parseValue());

            skipWhitespace();
            if (tryConsume(']')) {
                return array;
            }
            consume(',');
        }
    }

    std::string parseString() {
        consume('"');

        std::string result;
        while (!isAtEnd()) {
            const char current = advance();
            if (current == '"') {
                return result;
            }

            if (current == '\\') {
                if (isAtEnd()) {
                    throw error("Unterminated escape sequence");
                }

                const char escaped = advance();
                switch (escaped) {
                    case '"':
                    case '\\':
                    case '/':
                        result.push_back(escaped);
                        break;
                    case 'b':
                        result.push_back('\b');
                        break;
                    case 'f':
                        result.push_back('\f');
                        break;
                    case 'n':
                        result.push_back('\n');
                        break;
                    case 'r':
                        result.push_back('\r');
                        break;
                    case 't':
                        result.push_back('\t');
                        break;
                    case 'u':
                        throw error("Unicode escape sequences are not supported yet");
                    default:
                        throw error("Invalid escape sequence");
                }
            } else {
                result.push_back(current);
            }
        }

        throw error("Unterminated string");
    }

    std::size_t parseNumber() {
        std::size_t result = 0;

        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            const std::size_t digit = static_cast<std::size_t>(advance() - '0');
            result = (result * 10) + digit;
        }

        return result;
    }

    void skipWhitespace() {
        while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
            ++position_;
        }
    }

    void consume(char expected) {
        skipWhitespace();
        if (isAtEnd() || peek() != expected) {
            throw error(std::string("Expected '") + expected + "'");
        }
        ++position_;
    }

    bool tryConsume(char expected) {
        skipWhitespace();
        if (!isAtEnd() && peek() == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    char peek() const {
        return input_[position_];
    }

    char advance() {
        return input_[position_++];
    }

    bool isAtEnd() const {
        return position_ >= input_.size();
    }

    std::runtime_error error(const std::string& message) const {
        return std::runtime_error(message + " at byte " + std::to_string(position_));
    }

    std::string input_;
    std::size_t position_ = 0;
};

const JsonValue::Object& asObject(const JsonValue& value, const std::string& context) {
    if (const auto* object = std::get_if<JsonValue::Object>(&value.value)) {
        return *object;
    }

    throw std::runtime_error(context + " must be an object");
}

const JsonValue::Array& asArray(const JsonValue& value, const std::string& context) {
    if (const auto* array = std::get_if<JsonValue::Array>(&value.value)) {
        return *array;
    }

    throw std::runtime_error(context + " must be an array");
}

const std::string& asString(const JsonValue& value, const std::string& context) {
    if (const auto* string = std::get_if<std::string>(&value.value)) {
        return *string;
    }

    throw std::runtime_error(context + " must be a string");
}

std::size_t asNumber(const JsonValue& value, const std::string& context) {
    if (const auto* number = std::get_if<std::size_t>(&value.value)) {
        return *number;
    }

    throw std::runtime_error(context + " must be a non-negative integer");
}

const JsonValue& requireField(
    const JsonValue::Object& object,
    const std::string& field_name,
    const std::string& context
) {
    const auto field = object.find(field_name);
    if (field == object.end()) {
        throw std::runtime_error(context + " is missing required field '" + field_name + "'");
    }

    return field->second;
}

std::vector<std::string> readDependencies(const JsonValue::Object& job_object, const std::string& context) {
    const auto dependencies_field = job_object.find("dependencies");
    if (dependencies_field == job_object.end()) {
        return {};
    }

    std::vector<std::string> dependencies;
    const JsonValue::Array& dependency_values = asArray(dependencies_field->second, context + ".dependencies");
    for (std::size_t index = 0; index < dependency_values.size(); ++index) {
        dependencies.push_back(asString(
            dependency_values[index],
            context + ".dependencies[" + std::to_string(index) + "]"
        ));
    }

    return dependencies;
}

std::size_t readOptionalNumber(
    const JsonValue::Object& object,
    const std::string& field_name,
    const std::string& context,
    std::size_t default_value
) {
    const auto field = object.find(field_name);
    if (field == object.end()) {
        return default_value;
    }

    return asNumber(field->second, context + "." + field_name);
}

std::vector<Job> readJobs(const JsonValue& root) {
    const JsonValue::Object& root_object = asObject(root, "Root JSON value");
    const JsonValue::Array& job_values = asArray(requireField(root_object, "jobs", "Root JSON value"), "jobs");

    std::vector<Job> jobs;
    jobs.reserve(job_values.size());

    for (std::size_t index = 0; index < job_values.size(); ++index) {
        const std::string context = "jobs[" + std::to_string(index) + "]";
        const JsonValue::Object& job_object = asObject(job_values[index], context);

        Job job{
            asString(requireField(job_object, "name", context), context + ".name"),
            asString(requireField(job_object, "command", context), context + ".command"),
            readDependencies(job_object, context),
            readOptionalNumber(job_object, "max_retries", context, 0),
            std::chrono::milliseconds(readOptionalNumber(job_object, "retry_backoff_ms", context, 0))
        };

        if (job.name.empty()) {
            throw std::runtime_error(context + ".name must not be empty");
        }

        jobs.push_back(std::move(job));
    }

    return jobs;
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open config file: " + path);
    }

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

} // namespace

std::vector<Job> JobConfigLoader::loadJobsFromFile(const std::string& path) const {
    JsonParser parser(readFile(path));
    return readJobs(parser.parse());
}

} // namespace scheduler
