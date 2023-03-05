#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <stack>
#include <stdexcept>

//
// This sample uses fragments of code from simplereader.cpp and simplewriter.cpp
// from RapidJSON.
//

std::string make_json_with_writer()
{
   rapidjson::StringBuffer s;
   rapidjson::Writer<rapidjson::StringBuffer> writer(s);

   writer.StartObject();
   writer.Key("hello");
   writer.String("world");

   writer.Key("t");
   writer.Bool(true);

   writer.Key("f");
   writer.Bool(false);

   writer.Key("n");
   writer.Null();

   writer.Key("i");
   writer.Uint(123);

   writer.Key("pi");
   writer.Double(3.1416);

   // an array of integers
   writer.Key("a");
   writer.StartArray();
   for(unsigned int i = 0; i < 4; i++)
      writer.Uint(i);
   writer.EndArray();

   // an object
   writer.Key("o");
   writer.StartObject();
   writer.Key("a");
   writer.Uint64(12345);
   writer.Key("b");
   writer.String("XYZ");
   writer.Key("c");
   writer.StartArray();
   for(size_t i = 0; i < 3; i++)
      writer.String("X" + std::to_string(i));
   writer.EndArray();
   writer.EndObject();

   // array of objects
   writer.Key("ao");
   writer.StartArray();
   for(size_t i = 0; i < 2; i++) {
      writer.StartObject();
      writer.Key("i");
      writer.Uint(123);
      writer.Key("d");
      writer.Double(3.1416);
      writer.EndObject();
   }
   writer.EndArray();

   // array of arrays
   writer.Key("aa");
   writer.StartArray();
   for(size_t i = 0; i < 2; i++) {
      for(size_t k = 0; k < 2; k++) {
         writer.StartArray();
         writer.Uint(123);
         writer.Uint(456);
         writer.EndArray();
      }
   }
   writer.EndArray();

   writer.EndObject();

   return s.GetString();
}

struct ParseEventHandler {
   size_t indent = 0;

   // keeps track if we are in an array context
   std::stack<bool> array_context;

   // return a dash prefix for array values
   std::string value_prefix() { return !array_context.empty() && array_context.top() ? std::string(indent, ' ') + "- " : std::string(); }

   bool Null() { printf("%snull\n", value_prefix().c_str()); return true; }

   bool Bool(bool b) { printf("%s%s\n", value_prefix().c_str(), b ? "true" : "false"); return true; }

   bool Int(int i) { printf("%s%d\n", value_prefix().c_str(), i); return true; }
   bool Uint(unsigned u) { printf("%s%u\n", value_prefix().c_str(), u); return true; }
   bool Int64(int64_t i) { printf("%s%" PRIi64 "\n", value_prefix().c_str(), i); return true; }
   bool Uint64(uint64_t u) { printf("%s%" PRIu64 "\n", value_prefix().c_str(), u); return true; }
   bool Double(double d) { printf("%s%.3lf\n", value_prefix().c_str(), d); return true; }

   bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return String(str, length, copy); }

   bool String(const char* str, rapidjson::SizeType length, bool copy)
   {
      std::string s(str, length);
      printf("%s\"%s\"\n", value_prefix().c_str(), s.c_str());
      return true;
   }

   bool Key(const char* str, rapidjson::SizeType length, bool copy)
   {
      std::string s(str, length);
      std::string key_indent(indent, ' ');
      printf("%s%s: ", key_indent.c_str(), s.c_str());
      return true;
   }

   bool StartObject() { printf("%s\n", value_prefix().c_str()); indent += 2; array_context.push(false); return true; }
   bool EndObject(rapidjson::SizeType memberCount) { array_context.pop(); indent -= 2; return true; }

   bool StartArray() { printf("%s\n", value_prefix().c_str()); indent += 2; array_context.push(true); return true; }
   bool EndArray(rapidjson::SizeType elementCount) { array_context.pop(); indent -= 2; return true; }
};

void print_json_with_reader(const std::string& json)
{
   ParseEventHandler handler;
   rapidjson::Reader reader;
   rapidjson::StringStream ss(json.c_str());

   rapidjson::ParseResult parse_result = reader.Parse(ss, handler);

   if(parse_result.IsError())
      throw std::runtime_error(std::string("Parse error at ")
         + std::to_string(parse_result.Offset())
         + ": "
         + rapidjson::GetParseError_En(parse_result.Code()));
}

rapidjson::Document make_json_doc(const std::string& json)
{
   rapidjson::Document json_doc;

   json_doc.Parse(json);

   // using the ParseResult operator is the only way to get it from the document
   rapidjson::ParseResult parse_result = static_cast<rapidjson::ParseResult>(json_doc);

   if(static_cast<rapidjson::ParseResult>(json_doc).IsError())
      throw std::runtime_error(std::string("Parse error at ")
         + std::to_string(parse_result.Offset())
         + ": "
         + rapidjson::GetParseError_En(parse_result.Code()));

   return json_doc;
}

void print_json_value(const rapidjson::Value& json_value, size_t indent)
{
   std::string indent_value(indent, ' ');

   switch (json_value.GetType()) {
      case rapidjson::kNullType:
         printf("null\n");
         break;
      case rapidjson::kFalseType:
         printf("false\n");
         break;
      case rapidjson::kTrueType:
         printf("true\n");
         break;
      case rapidjson::kObjectType:
         printf("\n");
         for(rapidjson::Value::ConstMemberIterator i = json_value.MemberBegin(); i != json_value.MemberEnd(); i++) {
            printf("%s%s: ", indent_value.c_str(), i->name.GetString());
            print_json_value(i->value, indent + 2);
         }
         break;
      case rapidjson::kArrayType:
         printf("\n");
         for(rapidjson::Value::ConstValueIterator i = json_value.Begin(); i != json_value.End(); i++) {
            printf("%s- ", indent_value.c_str());
            print_json_value(*i, indent + 2);
         }
         break;
      case rapidjson::kStringType:
         printf("\"%s\"\n", json_value.GetString());
         break;
      case rapidjson::kNumberType:
         if(json_value.IsDouble())
            printf("%.3lf\n", json_value.GetDouble());
         else
            printf("%" PRId64 "\n", json_value.GetInt64());
         break;
      default:
         throw std::runtime_error("Unknown JSON Type (" + std::to_string(json_value.GetType()) + ")");
   }
}

void print_json_doc(rapidjson::Document&& json_doc)
{
   // move the document into a temporary, so it is destroyed after the call
   print_json_value(rapidjson::Document(std::move(json_doc)), 2);
}

int main(int argc, const char* argv[])
{
   try {
      // construct a JSON document in memory
      rapidjson::Document json_doc = make_json_doc(make_json_with_writer());

      printf("Printing a JSON document from memory\n");

      print_json_doc(std::move(json_doc));

      printf("\nPrinting a JSON document via a stream parser\n");

      // print JSON keys and values as they are being encountered by a stream parser
      print_json_with_reader(make_json_with_writer());

      try {
         //                            v-- missing comma, offset 12
         //                 1234567890123456789012345
         make_json_doc(R"==({"abc": 123 "def"; "XYZ"})==");
      }
      catch(const std::runtime_error& error) {
         printf("\nExpected: %s\n", error.what());
      }

      return EXIT_SUCCESS;
   }
   catch (const std::exception& error) {
      fprintf(stderr, "ERROR: %s\n", error.what());
   }
   catch (...) {
      fprintf(stderr, "ERROR: unknown\n");
   }

   return EXIT_FAILURE;
}
