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

#include "maidsafe-dht/tests/transport/test_transport_api.h"
#include <functional>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/thread.hpp"
#include "maidsafe-dht/transport/tcp_transport.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace bptime = boost::posix_time;

namespace maidsafe {

namespace transport {

namespace test {

TestMessageHandler::TestMessageHandler(const std::string &id)
    : this_id_(id),
      requests_received_(),
      responses_received_(),
      responses_sent_(),
      results_(),
      mutex_() {}

void TestMessageHandler::DoOnRequestReceived(const std::string &request,
                                             const Info &info,
                                             std::string *response,
                                             Timeout *timeout) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  boost::mutex::scoped_lock lock(mutex_);
  requests_received_.push_back(std::make_pair(request, info));
  *response = "Replied to " + request + " (Id = " + boost::lexical_cast<
              std::string>(requests_received_.size()) + ")";
  responses_sent_.push_back(*response);
  *timeout = kImmediateTimeout;
  DLOG(INFO) << this_id_ << " - Received request: \"" << request
              << "\".  Responding with \"" << *response << "\"" << std::endl;
}

void TestMessageHandler::DoOnResponseReceived(const std::string &request,
                                              const Info &info,
                                              std::string *response,
                                              Timeout *timeout) {
  response->clear();
  *timeout = kImmediateTimeout;
  boost::mutex::scoped_lock lock(mutex_);
  responses_received_.push_back(std::make_pair(request, info));
  DLOG(INFO) << this_id_ << " - Received response: \"" << request << "\""
              << std::endl;
}

void TestMessageHandler::DoOnError(const TransportCondition &tc) {
  boost::mutex::scoped_lock lock(mutex_);
  results_.push_back(tc);
  DLOG(INFO) << this_id_ << " - Error: " << tc << std::endl;
}

void TestMessageHandler::ClearContainers() {
  boost::mutex::scoped_lock lock(mutex_);
  requests_received_.clear();
  responses_received_.clear();
  responses_sent_.clear();
  results_.clear();
}

IncomingMessages TestMessageHandler::requests_received() {
  boost::mutex::scoped_lock lock(mutex_);
  return requests_received_;
}

IncomingMessages TestMessageHandler::responses_received() {
  boost::mutex::scoped_lock lock(mutex_);
  return responses_received_;
}

OutgoingResponses TestMessageHandler::responses_sent() {
  boost::mutex::scoped_lock lock(mutex_);
  return responses_sent_;
}

Results TestMessageHandler::results() {
  boost::mutex::scoped_lock lock(mutex_);
  return results_;
}


template <typename T>
TransportAPITest<T>::TransportAPITest()
    : asio_service_(new boost::asio::io_service),
      work_(new boost::asio::io_service::work(*asio_service_)),
      asio_service_1_(new boost::asio::io_service),
      work_1_(new boost::asio::io_service::work(*asio_service_1_)),
      asio_service_2_(new boost::asio::io_service),
      work_2_(new boost::asio::io_service::work(*asio_service_2_)),
      asio_service_3_(new boost::asio::io_service),
      work_3_(new boost::asio::io_service::work(*asio_service_3_)),
      count_(0),
      listening_transports_(),
      listening_message_handlers_(),
      sending_transports_(),
      sending_message_handlers_(),
      thread_group_(),
      thread_group_1_(),
      thread_group_2_(),
      thread_group_3_() {
  for (int i = 0; i < kThreadGroupSize; ++i)
    thread_group_.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service_));
  for (int i = 0; i < kThreadGroupSize; ++i)
    thread_group_1_.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service_1_));
  for (int i = 0; i < kThreadGroupSize; ++i)
    thread_group_2_.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service_2_));
  for (int i = 0; i < kThreadGroupSize; ++i)
    thread_group_3_.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service_3_));
  // count_ = 0;
}

template <typename T>
TransportAPITest<T>::~TransportAPITest() {
  work_.reset();
  work_1_.reset();
  work_2_.reset();
  work_3_.reset();
  asio_service_->stop();
  asio_service_1_->stop();
  asio_service_2_->stop();
  asio_service_3_->stop();
  thread_group_.join_all();
  thread_group_1_.join_all();
  thread_group_2_.join_all();
  thread_group_3_.join_all();
}

template <typename T>
void TransportAPITest<T>::SetupTransport(bool listen, Port lport) {
  if (listen) {
    TransportPtr transport1;
    if (count_ < 8)
      transport1 = TransportPtr(new T(asio_service_));
    else
      transport1 = TransportPtr(new T(asio_service_3_));

    if (lport != Port(0)) {
      EXPECT_EQ(kSuccess,
                transport1->StartListening(Endpoint(kIP, lport)));
    } else {
      while (kSuccess != transport1->StartListening(Endpoint(kIP,
                          (RandomUint32() % 60536) + 5000)));
    }  // do check for fail listening port
    listening_transports_.push_back(transport1);
  } else {
    TransportPtr transport1;
    if (count_ < 8)
      transport1 = TransportPtr(new T(asio_service_));
    else
      transport1 = TransportPtr(new T(asio_service_3_));
    sending_transports_.push_back(transport1);
  }
}

template <typename T>
void TransportAPITest<T>::RunTransportTest(const int &num_messages) {
  Endpoint endpoint;
  endpoint.ip = kIP;
  std::vector<TransportPtr>::iterator sending_transports_itr(
      sending_transports_.begin());
  while (sending_transports_itr != sending_transports_.end()) {
    TestMessageHandlerPtr msg_h(new TestMessageHandler("Sender"));
    (*sending_transports_itr)->on_message_received()->connect(boost::bind(
        &TestMessageHandler::DoOnResponseReceived, msg_h, _1, _2, _3, _4));
    (*sending_transports_itr)->on_error()->connect(
        boost::bind(&TestMessageHandler::DoOnError, msg_h, _1));
    sending_message_handlers_.push_back(msg_h);
    ++sending_transports_itr;
  }
  std::vector< TransportPtr >::iterator listening_transports_itr(
      listening_transports_.begin());
  while (listening_transports_itr != listening_transports_.end()) {
    TestMessageHandlerPtr msg_h(new TestMessageHandler("Receiver"));
    (*listening_transports_itr)->on_message_received()->connect(
        boost::bind(&TestMessageHandler::DoOnRequestReceived, msg_h, _1, _2,
                    _3, _4));
    (*listening_transports_itr)->on_error()->connect(boost::bind(
        &TestMessageHandler::DoOnError, msg_h, _1));
    listening_message_handlers_.push_back(msg_h);
    ++listening_transports_itr;
  }
  boost::uint16_t thread_size(0);
  sending_transports_itr = sending_transports_.begin();
  while (sending_transports_itr != sending_transports_.end()) {
    listening_transports_itr = listening_transports_.begin();
    while (listening_transports_itr != listening_transports_.end()) {
      for (int i = 0 ; i < num_messages; ++i) {
        if (thread_size > kThreadGroupSize) {
          // std::cout<<"\nthread_size::"<<thread_size<<std::endl;
          asio_service_2_->post(boost::bind(
            &transport::test::TransportAPITest<T>::SendRPC, this,
            *sending_transports_itr, *listening_transports_itr));
        } else {
          asio_service_1_->post(boost::bind(
            &transport::test::TransportAPITest<T>::SendRPC, this,
            *sending_transports_itr, *listening_transports_itr));
        }
        thread_size++;
      }
      ++listening_transports_itr;
    }
    ++sending_transports_itr;
  }
  boost::this_thread::sleep(boost::posix_time::seconds(10));
  work_.reset();
  work_1_.reset();
  work_2_.reset();
  work_3_.reset();
  asio_service_->stop();
  asio_service_1_->stop();
  asio_service_2_->stop();
  asio_service_3_->stop();
  thread_group_.join_all();
  thread_group_1_.join_all();
  thread_group_2_.join_all();
  thread_group_3_.join_all();
  CheckMessages();
  if (listening_message_handlers_.size() == 1) {
    std::vector<TestMessageHandlerPtr>::iterator sending_msg_handlers_itr(
        sending_message_handlers_.begin());
    while (sending_msg_handlers_itr != sending_message_handlers_.end()) {
    ASSERT_EQ((*sending_msg_handlers_itr)->responses_received().size(),
                num_messages);
    ++sending_msg_handlers_itr;
    }
  } else {
    std::vector<TestMessageHandlerPtr>::iterator sending_msg_handlers_itr(
        sending_message_handlers_.begin());
    while (sending_msg_handlers_itr != sending_message_handlers_.end()) {
      ASSERT_EQ((*sending_msg_handlers_itr)->responses_received().size(),
        listening_message_handlers_.size());
      ++sending_msg_handlers_itr;
    }
  }
  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  for (auto itr = listening_transports_.begin();
      itr < listening_transports_.end(); ++itr)
    (*itr)->StopListening();
  for (auto itr = sending_transports_.begin();
      itr < sending_transports_.end(); ++itr)
    (*itr)->StopListening();
}

template <typename T>
void TransportAPITest<T>::SendRPC(TransportPtr sender_pt,
                               TransportPtr listener_pt) {
  std::string request(RandomString(11));
  sender_pt->Send(request, Endpoint(kIP, listener_pt->listening_port()),
                  bptime::seconds(1));
  (request_messages_).push_back(request);
  // std::string response = base::RandomString(10); need to change
  std::string response("Response");
  listener_pt->Send(response, Endpoint(kIP, sender_pt->listening_port()),
                    bptime::seconds(1));
}

template <typename T>
void TransportAPITest<T>::CheckMessages() {
  bool found(false);
  // Compare Request
  std::vector<TestMessageHandlerPtr>::iterator listening_msg_handlers_itr(
      listening_message_handlers_.begin());
  while (listening_msg_handlers_itr != listening_message_handlers_.end()) {
    IncomingMessages listening_request_received =
        (*listening_msg_handlers_itr)->requests_received();
    IncomingMessages::iterator listening_request_received_itr =
        listening_request_received.begin();
    while (listening_request_received_itr !=
        listening_request_received.end()) {
      found = false;
      std::vector<std::string>::iterator request_messages_itr;
      request_messages_itr = std::find(
          request_messages_.begin(), request_messages_.end(),
          (*listening_request_received_itr).first);
      if (request_messages_itr != request_messages_.end())
        found = true;
      ++listening_request_received_itr;
    }
    if (!found)
      break;
    ++listening_msg_handlers_itr;
  }
  ASSERT_EQ(listening_msg_handlers_itr,
            listening_message_handlers_.end());

  // Compare Response
  std::vector<TestMessageHandlerPtr>::iterator sending_msg_handlers_itr(
      sending_message_handlers_.begin());
  listening_msg_handlers_itr = listening_message_handlers_.begin();
  while (sending_msg_handlers_itr != sending_message_handlers_.end()) {
    IncomingMessages sending_response_received =
        (*sending_msg_handlers_itr)->responses_received();
    IncomingMessages::iterator sending_response_received_itr =
        sending_response_received.begin();
    for (; sending_response_received_itr != sending_response_received.end();
        ++sending_response_received_itr) {
      found = false;
      listening_msg_handlers_itr = listening_message_handlers_.begin();
      while (listening_msg_handlers_itr !=
          listening_message_handlers_.end()) {
        OutgoingResponses listening_response_sent =
            (*listening_msg_handlers_itr)->responses_sent();
        OutgoingResponses::iterator listening_response_sent_itr;
        listening_response_sent_itr = std::find(
            listening_response_sent.begin(), listening_response_sent.end(),
            (*sending_response_received_itr).first);
        if (listening_response_sent_itr != listening_response_sent.end()) {
          found = true;
          break;
        }
        ++listening_msg_handlers_itr;
      }
      if (!found)
        break;
    }
    ASSERT_EQ(sending_response_received_itr,
              sending_response_received.end());
    ++sending_msg_handlers_itr;
  }
}


TYPED_TEST_P(TransportAPITest, BEH_TRANS_StartStopListening) {
  TransportPtr transport(new TypeParam(this->asio_service_));
  EXPECT_EQ(Port(0), transport->listening_port());
  EXPECT_EQ(kInvalidPort, transport->StartListening(Endpoint(kIP, 0)));
  EXPECT_EQ(kSuccess, transport->StartListening(Endpoint(kIP, 2277)));
  EXPECT_EQ(Port(2277), transport->listening_port());
  EXPECT_EQ(kAlreadyStarted, transport->StartListening(Endpoint(kIP, 2277)));
  EXPECT_EQ(kAlreadyStarted, transport->StartListening(Endpoint(kIP, 55123)));
  EXPECT_EQ(Port(2277), transport->listening_port());
  transport->StopListening();
  EXPECT_EQ(Port(0), transport->listening_port());
  EXPECT_EQ(kSuccess, transport->StartListening(Endpoint(kIP, 55123)));
  EXPECT_EQ(Port(55123), transport->listening_port());
  transport->StopListening();
  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_Send) {
  TransportPtr sender(new TypeParam(this->asio_service_));
  TransportPtr listener(new TypeParam(this->asio_service_));
  EXPECT_EQ(kSuccess, listener->StartListening(Endpoint(kIP, 2000)));
  TestMessageHandlerPtr msgh_sender(new TestMessageHandler("Sender"));
  TestMessageHandlerPtr msgh_listener(new TestMessageHandler("listener"));
  sender->on_message_received()->connect(
      boost::bind(&TestMessageHandler::DoOnResponseReceived, msgh_sender, _1,
      _2, _3, _4));
  sender->on_error()->connect(
      boost::bind(&TestMessageHandler::DoOnError, msgh_sender, _1));
  listener->on_message_received()->connect(
      boost::bind(&TestMessageHandler::DoOnRequestReceived, msgh_listener, _1,
      _2, _3, _4));
  listener->on_error()->connect(
      boost::bind(&TestMessageHandler::DoOnError, msgh_listener, _1));

  std::string request(RandomString(23));
  sender->Send(request, Endpoint(kIP, listener->listening_port()),
               bptime::seconds(1));
  boost::uint16_t timeout = 100;
  while (msgh_sender->responses_received().size() == 0 && timeout < 1100) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    timeout +=100;
  }
  EXPECT_GE(boost::uint16_t(1000), timeout);
  ASSERT_EQ(size_t(0), msgh_sender->results().size());
  ASSERT_EQ(size_t(1), msgh_listener->requests_received().size());
  ASSERT_EQ(request, msgh_listener->requests_received().at(0).first);
  ASSERT_EQ(size_t(1), msgh_listener->responses_sent().size());
  ASSERT_EQ(size_t(1), msgh_sender->responses_received().size());
  ASSERT_EQ(msgh_listener->responses_sent().at(0),
            msgh_sender->responses_received().at(0).first);

  // Timeout scenario
  request = RandomString(29);
  sender->Send(request, Endpoint(kIP, listener->listening_port()),
               bptime::milliseconds(2));
  timeout = 100;
  while (msgh_listener->requests_received().size() < 2 && timeout < 2000) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    timeout +=100;
  }
  ASSERT_EQ(size_t(1), msgh_sender->results().size());
  ASSERT_EQ(size_t(2), msgh_listener->requests_received().size());
  ASSERT_EQ(request, msgh_listener->requests_received().at(1).first);
  ASSERT_EQ(size_t(2), msgh_listener->responses_sent().size());
  ASSERT_EQ(size_t(1), msgh_sender->responses_received().size());
  listener->StopListening();
  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
}


TYPED_TEST_P(TransportAPITest, BEH_TRANS_OneToOneSingleMessage) {
  this->SetupTransport(false, 0);
  this->SetupTransport(true, 0);
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(1));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_OneToOneMultiMessage) {
  this->SetupTransport(false, 0);
  this->SetupTransport(true, 0);
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(20));
  boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_OneToManySingleMessage) {
  this->SetupTransport(false, 0);
  this->count_ = 0;
  for (int i = 0; i < 16; ++i) {
    this->SetupTransport(true, 0);
    this->count_++;
  }
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(1));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_OneToManyMultiMessage) {
  this->SetupTransport(false, 0);
  for (int i = 0; i < 10; ++i)
    this->SetupTransport(true, 0);
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(20));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_ManyToManyMultiMessage) {
  for (int i = 0; i < 15; ++i)
    this->SetupTransport(false, 0);
  for (int i = 0; i < 20; ++i)
    this->SetupTransport(true, 0);
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(2033));
}

TYPED_TEST_P(TransportAPITest, BEH_TRANS_Random) {
  boost::uint8_t num_sender_transports(
      static_cast<boost::uint8_t>(RandomUint32() % 10 + 5));
  boost::uint8_t num_listener_transports(
      static_cast<boost::uint8_t>(RandomUint32() % 10 + 5));
  boost::uint8_t num_messages(
      static_cast<boost::uint8_t>(RandomUint32() % 100 + 1));
  for (boost::uint8_t i = 0; i < num_sender_transports; ++i)
    this->SetupTransport(false, 0);
  for (boost::uint8_t i = 0; i < num_listener_transports; ++i)
    this->SetupTransport(true, 0);
  ASSERT_NO_FATAL_FAILURE(this->RunTransportTest(num_messages));
}

REGISTER_TYPED_TEST_CASE_P(TransportAPITest,
                           BEH_TRANS_StartStopListening,
                           BEH_TRANS_Send,
                           BEH_TRANS_OneToOneSingleMessage,
                           BEH_TRANS_OneToOneMultiMessage,
                           BEH_TRANS_OneToManySingleMessage,
                           BEH_TRANS_OneToManyMultiMessage,
                           BEH_TRANS_ManyToManyMultiMessage,
                           BEH_TRANS_Random);

INSTANTIATE_TYPED_TEST_CASE_P(TCP, TransportAPITest, TcpTransport);

}  // namespace test

}  // namespace transport

}  // namespace maidsafe
