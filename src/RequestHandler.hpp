#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "FileManager.hpp"
#include "PeerQueueManager.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class RequestHandler {
 private:
  PeerQueueManager& peerQueueManager;
  FileManager& fileManager;

 public:
  RequestHandler(PeerQueueManager& p, FileManager& f)
      : peerQueueManager(p), fileManager(f) {}

  void handleHeartbeat(tcp::socket& socket, const json& request) {
    peerQueueManager.updatePeerFiles(request["peer_ip"], request["files"]);
    json response = {{"status", "success"}, {"message", "Heartbeat received"}};
    std::cout << response << std::endl;
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  void handleUpload(tcp::socket& socket, const json& request) {
    std::string clientIP = socket.remote_endpoint().address().to_string();
    std::string fileName = request["file_name"];
    size_t fileSize = request["file_size"];
    std::string fileId = fileManager.saveFileMetadata(fileName, fileSize);
    const std::vector<std::string> fileIds = {fileId};
    peerQueueManager.updatePeerFiles(clientIP, fileIds);
    json response = {{"status", "success"},
                     {"message", "File uploaded successfully"},
                     {"uuid", fileId}};
    std::cout << response << std::endl;
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  void handleListFiles(tcp::socket& socket) {
    json response = {{"status", "success"}, {"files", fileManager.listFiles()}};
    std::cout << response << std::endl;
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  void handleFindFile(tcp::socket& socket, const json& request) {
    std::string fileId = request["file_id"];
    std::vector<std::string> peers = peerQueueManager.findPeersByFileId(fileId);
    json response = {{"status", peers.empty() ? "error" : "success"},
                     {"peers", peers},
                     {"message", peers.empty() ? "File not found on any peer"
                                               : "File found"}};
    std::cout << response << std::endl;
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  void handleDownload(tcp::socket& socket, const json& request) {
    // std::string fileId = request["file_id"];
    // std::string fileName = fileManager.getFileName(fileId);
    // std::ifstream file(fileName, std::ios::binary);
    // std::vector<std::string> peers =
    // peerQueueManager.findPeersByFileId(fileId); if (peers.empty()) {
    //   json response = {{"status", "error"}, {"message", "Peer not found"}};
    //   boost::asio::write(socket,
    //                      boost::asio::buffer(response.dump() + "<END>"));
    //   return;
    // }
    // if (!file.is_open()) {
    //   json response = {{"status", "error"}, {"message", "File not found"}};
    //   boost::asio::write(socket,
    //                      boost::asio::buffer(response.dump() + "<END>"));
    //   return;
    // }

    // char buffer[1024];
    // while (file.good()) {
    //   file.read(buffer, sizeof(buffer));
    //   std::streamsize bytesRead = file.gcount();
    //   boost::asio::write(socket, boost::asio::buffer(buffer, bytesRead));
    // }
    file.close();

    json response = {{"status", "success"},
                     {"ip", "1.233.64.47"},
                     {"message", "File downloaded successfully"}};
    std::cout << response << std::endl;
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  void handleListPeers(tcp::socket& socket) {
    json response = {{"status", "success"},
                     {"peers", peerQueueManager.listPeers()}};
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }
};

#endif  // REQUEST_HANDLER_HPP