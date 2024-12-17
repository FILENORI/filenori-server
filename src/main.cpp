#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

#include "FileManager.hpp"
#include "PeerQueueManager.hpp"
#include "RequestHandler.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class MainServer {
 private:
  boost::asio::io_context ioContext;
  tcp::acceptor acceptor;
  PeerQueueManager peerQueueManager;
  FileManager fileManager;
  RequestHandler requestHandler;

 public:
  MainServer(int port)
      : acceptor(ioContext, tcp::endpoint(tcp::v4(), port)),
        requestHandler(peerQueueManager, fileManager) {}

  void start() {
    std::cout << "Server running on port " << acceptor.local_endpoint().port()
              << "..." << std::endl;
    while (true) {
      tcp::socket socket(ioContext);
      acceptor.accept(socket);
      std::cout << "Client connected!" << std::endl;
      handleRequest(socket);
    }
  }

  void handleRequest(tcp::socket& socket) {
    char buffer[2048];
    boost::system::error_code error;

    while (true) {
      size_t bytesRead = socket.read_some(boost::asio::buffer(buffer), error);
      if (error == boost::asio::error::eof) break;
      std::string requestStr(buffer, bytesRead);
      json request = json::parse(requestStr);
      std::string action = request["action"];

      if (action == "heartbeat")
        requestHandler.handleHeartbeat(socket, request);
      else if (action == "upload")
        requestHandler.handleUpload(socket, request);
      else if (action == "list_files")
        requestHandler.handleListFiles(socket);
      else if (action == "find_file")
        requestHandler.handleFindFile(socket, request);
      else if (action == "download")
        requestHandler.handleDownload(socket, request);
      else if (action == "list_peers")
        requestHandler.handleListPeers(socket);
      else {
        json response = {{"status", "error"}, {"message", "Unknown action"}};
        boost::asio::write(socket,
                           boost::asio::buffer(response.dump() + "<END>"));
      }
    }
  }
};

int main() {
  try {
    MainServer server(12345);
    server.start();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
