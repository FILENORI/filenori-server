#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using boost::asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;

// 파일 업로드 처리
void handle_upload(tcp::socket& socket, const json& request_json) {
  std::string file_name = request_json["file_name"];
  std::size_t file_size = request_json["file_size"];

  std::ofstream file(file_name, std::ios::binary);
  std::cout << "testtest" << std::endl;
  std::cout << file_name << std::endl;
  if (!file.is_open()) {
    json response = {{"status", "error"},
                     {"message", "Failed to open file for writing"}};
    boost::asio::write(socket, boost::asio::buffer(response.dump()));
    return;
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
  json response = {{"status", "success"}, {"message", "File uploaded"}};
  boost::asio::write(socket, boost::asio::buffer(response.dump()));
}

// 파일 다운로드 처리
void handle_download(tcp::socket& socket, const json& request_json) {
  std::string file_name = request_json["file_name"];
  std::string file_path = "./" + file_name;  // 현재 디렉토리 기준
  std::cout << file_path << std::endl;
  std::ifstream file(file_path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "File not found: " << file_name << std::endl;
    json response = {{"status", "error"}, {"message", "File not found"}};
    boost::asio::write(socket, boost::asio::buffer(response.dump()));
    return;
  }

  char buffer[1024];
  while (file.good()) {
    file.read(buffer, sizeof(buffer));
    std::streamsize bytes_read = file.gcount();
    boost::asio::write(socket, boost::asio::buffer(buffer, bytes_read));
  }

  file.close();
  std::cout << "File downloaded: " << file_name << std::endl;
}

// 파일 목록 조회 처리
void handle_list_files(tcp::socket& socket) {
  json response = {{"status", "success"}, {"files", json::array()}};

  for (const auto& entry : fs::directory_iterator(".")) {
    if (entry.is_regular_file()) {
      response["files"].push_back({{"name", entry.path().filename().string()},
                                   {"size", fs::file_size(entry)}});
    }
  }

  boost::asio::write(socket, boost::asio::buffer(response.dump()));
}

// 클라이언트 요청 처리
void handle_request(tcp::socket& socket) {
  try {
    char buffer[2048];
    size_t bytes_read = socket.read_some(boost::asio::buffer(buffer));
    std::string request_str(buffer, bytes_read);

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
      boost::asio::write(socket, boost::asio::buffer(response.dump()));
    }
  } catch (const std::exception& e) {
    std::cerr << "Error handling request: " << e.what() << std::endl;
    json response = {{"status", "error"}, {"message", e.what()}};
    boost::asio::write(socket, boost::asio::buffer(response.dump()));
  }
}

// 메인 서버 실행
int main() {
  try {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

    std::cout << "Server running on port 12345..." << std::endl;

    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::cout << "Client connected!" << std::endl;
      handle_request(socket);
    }
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << std::endl;
  }

  return 0;
}
