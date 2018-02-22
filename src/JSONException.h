#pragma once
#ifndef DVJSON_JSONEXCEPTION_H
#define DVJSON_JSONEXCEPTION_H

#include <stdexcept>
#include <string>

namespace dv {
  namespace json {
    class JSONException : public std::exception {
     public:
      JSONException();
      JSONException( const std::string &message );
      JSONException( const char *message );
      JSONException( const JSONException & ) = default;
      JSONException( JSONException && ) = default;
      ~JSONException() override;
      const char *what() const noexcept override;

     protected:
      std::string msg;
    };

    class JSONParseException : public JSONException {
    public:
      JSONParseException( const std::string &nMessage );
      JSONParseException( const char *nMessage );
      JSONParseException( const JSONParseException & ) = default;
      JSONParseException( JSONParseException && ) = default;
    };

    class JSONWrongTypeException : public JSONException {
    public:
      JSONWrongTypeException( const std::type_info &nExpected, const std::type_info &nActual );
      JSONWrongTypeException( const JSONWrongTypeException & ) = default;
      JSONWrongTypeException( JSONWrongTypeException && ) = default;
      const std::type_info &getExpected() const;
      const std::type_info &getActual() const;

    protected:
      const std::type_info &expected;
      const std::type_info &actual;
    };

    class JSONNoSuchKeyException : public JSONException {
    public:
      JSONNoSuchKeyException( std::size_t index );
      JSONNoSuchKeyException( const char *nKey );
      JSONNoSuchKeyException( const std::string &nKey );
      JSONNoSuchKeyException( const JSONNoSuchKeyException & ) = default;
      JSONNoSuchKeyException( JSONNoSuchKeyException && ) = default;

      inline const std::string &getKey() const { return key; }

    protected:
      std::string key;
    };
  }
}

#endif //DVJSON_JSONEXCEPTION_H
