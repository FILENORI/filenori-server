#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include "PeerQueueManager.hpp"
#include "FileManager.hpp"
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

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
        boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
    }

    void handleUpload(tcp::socket& socket, const json& request) {
        std::string fileName = request["file_name"];
        size_t fileSize = request["file_size"];
        std::string fileId = fileManager.saveFileMetadata(fileName, fileSize);
        json response = {{"status", "success"}, {"message", "File uploaded successfully"}, {"uuid", fileId}};
        boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
    }

    void handleListPeers(tcp::socket& socket) {
        json response = {{"status", "success"}, {"peers", peerQueueManager.listPeers()}};
        boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
    }
};

#endif // REQUEST_HANDLER_HPP