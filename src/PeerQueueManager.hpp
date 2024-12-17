#ifndef PEER_QUEUE_MANAGER_HPP
#define PEER_QUEUE_MANAGER_HPP

#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

using json = nlohmann::json;

class PeerQueueManager {
private:
    std::map<std::string, std::vector<std::string>> peerFileMap;

public:
    void updatePeerFiles(const std::string& peer_ip, const std::vector<std::string>& files) {
        peerFileMap[peer_ip] = files;
    }

    std::vector<std::string> findPeersByFileId(const std::string& fileId) const {
        std::vector<std::string> peers;
        for (const auto& [peer, files] : peerFileMap) {
            if (std::find(files.begin(), files.end(), fileId) != files.end()) {
                peers.push_back(peer);
            }
        }
        return peers;
    }

    json listPeers() const {
        json result = json::array();
        for (const auto& [peer, files] : peerFileMap) {
            result.push_back({{"peer", peer}, {"file_count", files.size()}});
        }
        return result;
    }
};

#endif // PEER_QUEUE_MANAGER_HPP