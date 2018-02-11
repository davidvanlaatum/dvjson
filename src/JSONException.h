#pragma once
#ifndef DVFORMSCPP_JSONEXCEPTION_H
#define DVFORMSCPP_JSONEXCEPTION_H

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
  }
}

#endif //DVFORMSCPP_JSONEXCEPTION_H
