#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpc/grpc.h>

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "sfs.grpc.pb.h"

#include <dirent.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using sfs::DirEnt;
using sfs::EntKind;
using sfs::ListDirReq;

class SfsServiceImpl final : public sfs::Sfs::Service {
    Status ListDir(ServerContext* context, const ListDirReq* req,
                   ServerWriter<DirEnt>* writer) override {
        DIR *dir;
        struct dirent *ent;
        
        dir = opendir(req->path().c_str());
        if (!dir) {
            return Status::CANCELLED;
        }

        /* TODO skip erroneous entries */
        while ((ent = readdir(dir)) != NULL) {
            DirEnt ent_proto;

            std::string file_name(ent->d_name);
            EntKind type = EntKind::ENT_KIND_UNKNOWN;

#ifdef _DIRENT_HAVE_D_TYPE
            switch (ent->d_type) {
                case DT_REG:
                    type = EntKind::ENT_KIND_FILE;
                break;

                case DT_DIR:
                    type = EntKind::ENT_KIND_DIRECTORY;
                break;
            }
#endif
            ent_proto.set_path(file_name);
            ent_proto.set_type(type);

            writer->Write(ent_proto);
        }

        closedir(dir);

        return Status::OK;
    }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  SfsServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
