#ifndef PEER_QUEUE_MANAGER_HPP
#define PEER_QUEUE_MANAGER_HPP

#include <algorithm>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class PeerQueueManager {
 private:
  std::map<std::string, std::vector<std::string>> peerFileMap;

 public:
  void updatePeerFiles(const std::string& peer_ip,
                       const std::vector<std::string>& files) {
    std::cout << "Updating peer files for: " << peer_ip << std::endl;
    for (const auto& file : files) {
      std::cout << "File: " << file << std::endl;
    }
    peerFileMap[peer_ip] = files;
  }

  std::vector<std::string> findPeersByFileId(const std::string& fileId) const {
    std::vector<std::string> peers;
    for (const auto& [peer, files] : peerFileMap) {
      for (const auto& file : files) {
        std::cout << "Checking file: " << file << " against fileId: " << fileId
                  << std::endl;
        if (file == fileId) {
          peers.push_back(peer);
          break;  // 일치하면 더 이상 확인하지 않음
        }
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

#endif  // PEER_QUEUE_MANAGER_HPP