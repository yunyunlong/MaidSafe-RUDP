/* Copyright (c) 2010 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_KADEMLIA_MESSAGEHANDLER_H_
#define MAIDSAFE_KADEMLIA_MESSAGEHANDLER_H_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/signals2/signal.hpp>

#include "maidsafe/transport/messagehandler.h"
#include "maidsafe/kademlia/rpcs.pb.h"

namespace bs2 = boost::signals2;

namespace kademlia {

class MessageHandler : public transport::MessageHandler {
 public:
  const int kMessageTypeExt;  // Offset for type extensions.
  typedef boost::shared_ptr<bs2::signal<void(protobuf::PingRequest,
      protobuf::PingResponse*)> > PingReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::PingResponse)> >
      PingRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::FindValueRequest,
      protobuf::FindValueResponse*)> > FindValueReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::FindValueResponse)> >
      FindValueRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::FindNodesRequest,
      protobuf::FindNodesResponse*)> > FindNodesReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::FindNodesResponse)> >
      FindNodesRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::StoreRequest,
      protobuf::StoreResponse*)> > StoreReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::StoreResponse)> >
      StoreRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::DeleteRequest,
      protobuf::DeleteResponse*)> > DeleteReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::DeleteResponse)> >
      DeleteRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::UpdateRequest,
      protobuf::UpdateResponse*)> > UpdateReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::UpdateResponse)> >
      UpdateRspSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::DownlistRequest,
      protobuf::DownlistResponse*)> > DownlistReqSigPtr;
  typedef boost::shared_ptr<bs2::signal<void(protobuf::DownlistResponse)> >
      DownlistRspSigPtr;
   
  MessageHandler()
    : kMessageTypeExt(transport::MessageHandler::kMessageTypeExt + 14),
      on_ping_request_(new PingReqSigPtr::element_type),
      on_ping_response_(new PingRspSigPtr::element_type),
      on_find_value_request_(new FindValueReqSigPtr::element_type),
      on_find_value_response_(new FindValueRspSigPtr::element_type),
      on_find_nodes_request_(new FindNodesReqSigPtr::element_type),
      on_find_nodes_response_(new FindNodesRspSigPtr::element_type),
      on_store_request_(new StoreReqSigPtr::element_type),
      on_store_response_(new StoreRspSigPtr::element_type),
      on_delete_request_(new DeleteReqSigPtr::element_type),
      on_delete_response_(new DeleteRspSigPtr::element_type),
      on_update_request_(new UpdateReqSigPtr::element_type),
      on_update_response_(new UpdateRspSigPtr::element_type),
      on_downlist_request_(new DownlistReqSigPtr::element_type),
      on_downlist_response_(new DownlistRspSigPtr::element_type) {}
  virtual ~MessageHandler() {}

  std::string WrapMessage(const protobuf::PingRequest &msg);
  std::string WrapMessage(const protobuf::PingResponse &msg);
  std::string WrapMessage(const protobuf::FindValueRequest &msg);
  std::string WrapMessage(const protobuf::FindValueResponse &msg);
  std::string WrapMessage(const protobuf::FindNodesRequest &msg);
  std::string WrapMessage(const protobuf::FindNodesResponse &msg);
  std::string WrapMessage(const protobuf::StoreRequest &msg);
  std::string WrapMessage(const protobuf::StoreResponse &msg);
  std::string WrapMessage(const protobuf::DeleteRequest &msg);
  std::string WrapMessage(const protobuf::DeleteResponse &msg);
  std::string WrapMessage(const protobuf::UpdateRequest &msg);
  std::string WrapMessage(const protobuf::UpdateResponse &msg);
  std::string WrapMessage(const protobuf::DownlistRequest &msg);
  std::string WrapMessage(const protobuf::DownlistResponse &msg);
  
  PingReqSigPtr on_ping_request() { return on_ping_request_; }
  PingRspSigPtr on_ping_response() { return on_ping_response_; }
  FindValueReqSigPtr on_find_value_request() { return on_find_value_request_; }
  FindValueRspSigPtr on_find_value_response() {
    return on_find_value_response_;
  }
  FindNodesReqSigPtr on_find_nodes_request() { return on_find_nodes_request_; }
  FindNodesRspSigPtr on_find_nodes_response() {
    return on_find_nodes_response_;
  }
  StoreReqSigPtr on_store_request() { return on_store_request_; }
  StoreRspSigPtr on_store_response() { return on_store_response_; }
  DeleteReqSigPtr on_delete_request() { return on_delete_request_; }
  DeleteRspSigPtr on_delete_response() { return on_delete_response_; }
  UpdateReqSigPtr on_update_request() { return on_update_request_; }
  UpdateRspSigPtr on_update_response() { return on_update_response_; }
  DownlistReqSigPtr on_downlist_request() { return on_downlist_request_; }
  DownlistRspSigPtr on_downlist_response() { return on_downlist_response_; }
 protected:
  virtual void ProcessSerialisedMessage(const int &message_type,
                                        const std::string &payload,
                                        std::string *response,
                                        transport::Timeout *timeout);
 private:
  MessageHandler(const MessageHandler&);
  MessageHandler& operator=(const MessageHandler&);
  PingReqSigPtr on_ping_request_;
  PingRspSigPtr on_ping_response_;
  FindValueReqSigPtr on_find_value_request_;
  FindValueRspSigPtr on_find_value_response_;
  FindNodesReqSigPtr on_find_nodes_request_;
  FindNodesRspSigPtr on_find_nodes_response_;
  StoreReqSigPtr on_store_request_;
  StoreRspSigPtr on_store_response_;
  DeleteReqSigPtr on_delete_request_;
  DeleteRspSigPtr on_delete_response_;
  UpdateReqSigPtr on_update_request_;
  UpdateRspSigPtr on_update_response_;
  DownlistReqSigPtr on_downlist_request_;
  DownlistRspSigPtr on_downlist_response_;
};

}  // namespace kademlia

#endif  // MAIDSAFE_KADEMLIA_MESSAGEHANDLER_H_