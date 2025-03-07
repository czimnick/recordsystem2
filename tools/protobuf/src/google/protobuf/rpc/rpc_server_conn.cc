// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_server_conn.h"
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_wire.h>
#include <google/protobuf/stubs/defer.h>

namespace google {
namespace protobuf {
namespace rpc {

ServerConn::ServerConn(Server* server, Conn* conn, Env* env):
  server_(server), conn_(conn), env_(env) {
  //
}
ServerConn::~ServerConn() {
  conn_->Close();
  delete conn_;
}

void ServerConn::Serve(Server* server, Conn* conn, Env* env) {
  auto self = new ServerConn(server, conn, env);
  env->Schedule(ServerConn::ServeProc, self);
}

// [static]
void ServerConn::ServeProc(void* p) {
  auto self = (ServerConn*)p;

  if(self->env_->Handshake(self->conn_)) {
    self->env_->Logf("client %s handshake done!\n", self->conn_->RemoteIpAdress());
    for(;;) {
      auto err = self->ProcessOneCall(self->conn_);
      if(!err.IsNil()) {
        self->env_->Logf("%s\n", err.String().c_str());
        break;
      }
    }
  }else
	self->env_->Logf("ERROR: client %s handshake faild!\n", self->conn_->RemoteIpAdress());

  self->env_->ClientDisconnect(self->conn_);
  delete self;
}

Error ServerConn::ProcessOneCall(Conn* receiver) {
  wire::RequestHeader reqHeader;
  Error err;

  // 1. recv request header
  err = RecvRequestHeader(receiver, &reqHeader);
  if(!err.IsNil()) {
    return err;
  }

  // 2. find service/method
  auto method = server_->FindMethodDescriptor(reqHeader.method());
  if(method == NULL) {
    wire::SendResponse(receiver, reqHeader.id(),
      "Can't find ServiceMethod: " + reqHeader.method(),
       NULL
    );
    return Error::Nil();
  }
  auto service = server_->FindService(reqHeader.method());
  if(service == NULL) {
    wire::SendResponse(receiver, reqHeader.id(),
      "Can't find ServiceMethod: " + reqHeader.method(),
       NULL
    );
    return Error::Nil();
  }

  // 3. make request/response message
  auto request = service->GetRequestPrototype(method).New();
  auto response = service->GetResponsePrototype(method).New();

  // 4. recv request body
  err = wire::RecvRequestBody(receiver, &reqHeader, request);
  if(!err.IsNil()) {
    env_->Logf("ServerConn: RecvRequestBody fail: %s.\n", err.String().c_str());
    delete request; delete response;
    return err;
  }

  // 5. call method
  request->TagObj = receiver;
  response->TagObj = receiver;
  auto rv = service->CallMethod(method, request, response);
  request->TagObj = NULL;
  response->TagObj = NULL;

  // 6. send response
  err = wire::SendResponse(receiver, reqHeader.id(), rv.String(), response);
  if(!err.IsNil()) {
    env_->Logf("ServerConn: SendResponse fail: %s.\n", err.String().c_str());
    delete request; delete response;
    return err;
  }

  delete request; delete response;
  return Error::Nil();
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

