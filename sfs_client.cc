#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "sfs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using sfs::Sfs;
using sfs::ListDirReq;
using sfs::DirEnt;
using sfs::EntKind;

class SfsClient {
private:
    std::unique_ptr<Sfs::Stub> stub_;

public:
    SfsClient(std::shared_ptr<Channel> channel)
        : stub_(Sfs::NewStub(channel)) {}

     void ListDir(const std::string& path) {
        ClientContext context;
        ListDirReq req;
        DirEnt ent;

        req.set_path(path);

        std::unique_ptr<ClientReader<DirEnt>> reader(stub_->ListDir(&context, req));
        
        while (reader->Read(&ent)) {
            switch (ent.type()) {
                case EntKind::ENT_KIND_FILE:
                    std::cout << "-\t" << ent.path() << std::endl;
                break;
                case EntKind::ENT_KIND_DIRECTORY:
                    std::cout << "d\t" << ent.path() << std::endl;
                break;
                default:
                    std::cout << "?\t" << ent.path() << std::endl;
                break;
            }
        }

        Status status = reader->Finish();

        if (!status.ok())
            std::cerr << "Couldn't list directories!" << std::endl;
    }
};

int main(int argc, char** argv)
{
    SfsClient client(
      grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    client.ListDir(".");
}
