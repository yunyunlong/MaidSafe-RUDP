/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

// Original author: Christopher M. Kohlhoff (chris at kohlhoff dot com)

#ifndef MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_
#define MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_

#include "boost/asio/handler_alloc_hook.hpp"
#include "boost/asio/handler_invoke_hook.hpp"
#include "boost/system/error_code.hpp"

namespace maidsafe {

namespace rudp {

namespace detail {

// Helper class to adapt a probe handler into a waiting operation.
template <typename ProbeHandler>
class ProbeOp {
 public:
  ProbeOp(ProbeHandler handler, const boost::system::error_code& ec)
      : handler_(handler),
        ec_(ec) {}

  ProbeOp(const ProbeOp& other) : handler_(other.handler_), ec_(other.ec_) {}

  void operator()(boost::system::error_code ec) {
    if (boost::asio::error::timed_out == ec)
      handler_(ec);
    else
      handler_(ec_);
  }

  friend void* asio_handler_allocate(size_t n, ProbeOp* op) {
    using boost::asio::asio_handler_allocate;
    return asio_handler_allocate(n, &op->handler_);
  }

  friend void asio_handler_deallocate(void* p, size_t n, ProbeOp* op) {
    using boost::asio::asio_handler_deallocate;
    asio_handler_deallocate(p, n, &op->handler_);
  }

  template <typename Function>
  friend void asio_handler_invoke(const Function& f, ProbeOp* op) {
    using boost::asio::asio_handler_invoke;
    asio_handler_invoke(f, &op->handler_);
  }

 private:
  // Disallow assignment.
  ProbeOp& operator=(const ProbeOp&);

  ProbeHandler handler_;
  const boost::system::error_code& ec_;
};

}  // namespace detail

}  // namespace rudp

}  // namespace maidsafe

#endif  // MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_
