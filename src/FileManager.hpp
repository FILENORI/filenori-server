#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

class FileManager {
 private:
  const std::string metadataFile = "file_metadata.json";
  std::map<std::string, std::string> fileIdToName;

  std::string generateUUID() const {
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
  }

  void writeFileMetadata(const json& metadata) const {
    std::ofstream file(metadataFile);
    if (!file.is_open()) throw std::runtime_error("Cannot open metadata file");
    file << metadata.dump(2);
  }

  json readFileMetadata() const {
    std::ifstream file(metadataFile);
    if (!file.is_open()) return json::array();

    json metadata;
    file >> metadata;
    return metadata;
  }

 public:
  std::string saveFileMetadata(const std::string& fileName, size_t fileSize) {
    std::string uuid = generateUUID();
    json metadata = readFileMetadata();
    metadata.push_back(
        {{"name", fileName}, {"size", fileSize}, {"uuid", uuid}});
    writeFileMetadata(metadata);
    fileIdToName[uuid] = fileName;
    return uuid;
  }

  std::string getFileName(const std::string& fileId) const {
    if (fileIdToName.find(fileId) == fileIdToName.end())
      throw std::runtime_error("Invalid file ID");
    return fileIdToName.at(fileId);
  }
  json listFiles() const { return readFileMetadata(); }
};

#endif  // FILE_MANAGER_HPP