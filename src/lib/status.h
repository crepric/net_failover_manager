// This file is part of Net Failover Manager.
//
// Net Failover Manager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Net Failover Manager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Net Failover Manager.  If not, see
// <https://www.gnu.org/licenses/>.

#ifndef NET_FAILOVER_MANAGER_LIB_STATUS
#define NET_FAILOVER_MANAGER_LIB_STATUS

#include <string>

namespace net_failover_manager {

// This class can be used to return more informative errors from a function,
// rather than a simple boleean value.
// This class is thread safe. The only changes allowed are at construction.
class Status {
public:
  typedef enum {
    OK,                // Operation successful.
    NO_OP,             // No operation was necessary.
    UNKNOWN_ERROR,     // Unknown failure.
    NOT_FOUND,         // Argument not found.
    NOT_IMPLEMENTED,   // Function not implemented.
    PERMISSION_ERROR,  // Operation denied due to permissions.
    INVALID_ARGUMENTS, // The caller used invalid arguments.
  } ErrorCode;

  // Constructor for a Status object.
  // Args:
  //   error_code: one of the possible statuses defined in "ErrorCode"
  //   message: a string describing the error condition.
  Status(ErrorCode error_code, const std::string &message)
      : error_(error_code), message_(message){};

  // Constructs an object that can be used to signal a successful result.
  static Status Ok() { return Status(OK, ""); }

  // Copy constructors.
  Status(const Status &other) {
    error_ = other.Error();
    message_ = other.ErrorMessage();
  }

  Status &operator=(const Status &other) {
    if (this == &other) {
      return *this;
    }
    error_ = other.Error();
    message_ = other.ErrorMessage();
    return *this;
  }

  // Accessors for the error code and the error message.
  ErrorCode Error() const { return error_; }
  const std::string &ErrorMessage() const { return message_; }

protected:
  Status() = delete;

private:
  ErrorCode error_;
  std::string message_;
};

} // namespace net_failover_manager

#endif // NET_FAILOVER_MANAGER_LIB_STATUS
