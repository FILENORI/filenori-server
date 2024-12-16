#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using boost::asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;

// 업로드된 파일 정보가 저장될 JSON 파일 경로
const std::string FILE_METADATA_JSON = "file_metadata.json";

// JSON 파일에서 메타데이터 읽기
json read_file_metadata() {
  std::ifstream file(FILE_METADATA_JSON);
  if (!file.is_open()) {
    return json::array();  // JSON 파일이 없으면 빈 배열 반환
  }
  json metadata;
  file >> metadata;
  return metadata;
}

// JSON 파일에 메타데이터 저장
void write_file_metadata(const json& metadata) {
  std::ofstream file(FILE_METADATA_JSON, std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "Failed to open metadata file for writing" << std::endl;
    return;
  }
  file << metadata.dump(4);  // 예쁘게 저장
}

// 파일 업로드 처리
void handle_upload(tcp::socket& socket, const json& request_json) {
  std::string file_name = request_json["file_name"];
  std::size_t file_size = request_json["file_size"];

  std::ofstream file(file_name, std::ios::binary);
  if (!file.is_open()) {
    json response = {{"status", "error"},
                     {"message", "Failed to open file for writing"}};
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
    return;
  } else {
    json response = {{"status", "success"}, {"message", "get metadata"}};
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
  }

  char buffer[1024];
  std::size_t bytes_received = 0;

  while (bytes_received < file_size) {
    boost::system::error_code error;
    size_t len = socket.read_some(boost::asio::buffer(buffer), error);

    if (error == boost::asio::error::eof) {
      break;
    } else if (error) {
      throw boost::system::system_error(error);
    }

    file.write(buffer, len);
    bytes_received += len;
  }
  file.close();

  // 파일 메타데이터 저장
  json metadata = read_file_metadata();
  metadata.push_back({{"name", file_name}, {"size", file_size}});
  write_file_metadata(metadata);

  json response = {{"status", "success"},
                   {"message", "File uploaded successfully"}};
  boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
}

// 파일 목록 조회 처리
void handle_list_files(tcp::socket& socket) {
  json metadata = read_file_metadata();

  json response = {{"status", "success"}, {"files", metadata}};
  boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
}

// 파일 다운로드 처리
void handle_download(tcp::socket& socket, const json& request_json) {
  std::string file_name = request_json["file_name"];

  std::ifstream file(file_name, std::ios::binary);
  if (!file.is_open()) {
    json response = {{"status", "error"}, {"message", "File not found"}};
    boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
    return;
  }

  char buffer[1024];
  while (file.good()) {
    file.read(buffer, sizeof(buffer));
    std::streamsize bytes_read = file.gcount();

    boost::asio::write(socket, boost::asio::buffer(buffer, bytes_read));
  }

  file.close();
  json response = {{"status", "success"},
                   {"message", "File downloaded successfully"}};
  boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
}

// 요청 처리
void handle_request(tcp::socket& socket) {
  while (true) {
    char buffer[2048];
    boost::system::error_code error;

    // 클라이언트 요청 읽기
    size_t bytes_read = socket.read_some(boost::asio::buffer(buffer), error);

    if (error == boost::asio::error::eof) {
      std::cout << "Connection closed by client" << std::endl;
      break;
    } else if (error) {
      throw boost::system::system_error(error);
    }

    std::string request_str(buffer, bytes_read);
    std::cout << "Received request: " << request_str << std::endl;

    try {
      json request_json = json::parse(request_str);
      std::string action = request_json["action"];

      if (action == "upload") {
        handle_upload(socket, request_json);
      } else if (action == "download") {
        handle_download(socket, request_json);
      } else if (action == "list_files") {
        handle_list_files(socket);
      } else {
        json response = {{"status", "error"}, {"message", "Unknown action"}};
        boost::asio::write(socket,
                           boost::asio::buffer(response.dump() + "<END>"));
      }
    } catch (const std::exception& ex) {
      std::cerr << "Error handling request: " << ex.what() << std::endl;
      json response = {{"status", "error"}, {"message", "Invalid request"}};
      boost::asio::write(socket,
                         boost::asio::buffer(response.dump() + "<END>"));
    }
  }
}

// 메인 서버 실행
int main() {
  boost::asio::io_context io_context;
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

  std::cout << "Server running on port 12345..." << std::endl;

  while (true) {
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    std::cout << "Client connected!" << std::endl;
    handle_request(socket);
  }

  return 0;
}
