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

  std::cout << "file_size" << file_size << std::endl;
  while (bytes_received < file_size) {
    std::cout << "upload start" << std::endl;
    boost::system::error_code error;
    std::cout << "A" << std::endl;
    size_t len = socket.read_some(boost::asio::buffer(buffer), error);
    std::cout << len << std::endl;

    if (error == boost::asio::error::eof) {
      std::cout << "upload break" << std::endl;
      break;
    } else if (error) {
      throw boost::system::system_error(error);
    }

    std::cout << "file write start" << std::endl;
    file.write(buffer, len);
    std::cout << "file write end" << std::endl;
    bytes_received += len;
    std::cout << "bytes_received" << bytes_received << std::endl;
  }
  std::cout << "while end" << std::endl;
  file.close();
  std::cout << "upload success" << std::endl;
  json response = {{"status", "success"},
                   {"message", "File uploaded successfully"}};
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

// 파일 목록 조회 처리
void handle_list_files(tcp::socket& socket) {
  std::cout << "get file list" << std::endl;
  json response = {{"status", "success"}, {"files", json::array()}};

  for (const auto& entry : fs::directory_iterator(".")) {
    if (entry.is_regular_file()) {
      std::cout << entry.path().filename().string() << fs::file_size(entry)
                << std::endl;
      response["files"].push_back({{"name", entry.path().filename().string()},
                                   {"size", fs::file_size(entry)}});
    }
  }

  boost::asio::write(socket, boost::asio::buffer(response.dump() + "<END>"));
}
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
