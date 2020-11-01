/*!
 * @file AsioUDP 基于boost::asio封装UDP通信
 * @version 0.1
 * @date May 23, 2017
 * @note
 * - 基于boost::asio封装实现UDP通信服务器和客户端
 * @version 0.2
 * @date Oct 30, 2020
 * @note
 * - 优化
 */

#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include "AsioUDP.h"

using namespace boost::system;
using namespace boost::asio;
using namespace boost::placeholders;

AsioUDP::AsioUDP()
	: sock_(keep_.GetIOService()) {
	connected_  = false;
	byte_read_  = 0;
	buf_read_.reset(new char[UDP_PACK_SIZE]);
	block_reading_ = false;
}

AsioUDP::~AsioUDP() {
	Close();
}

bool AsioUDP::Open(uint16_t port, bool v6) {
	try {
		if (!port) sock_.open(v6 ? UDP::v6() : UDP::v4());
		else {// 在端口启动UDP接收服务
			UDP::endpoint end(v6 ? UDP::v6() : UDP::v4(), port);
			sock_.open(end.protocol());
			sock_.bind(end);
			start_read();
		}
	}
	catch(error_code &ec) {
		return false;
	}
	return true;
}

void AsioUDP::Close() {
	if (sock_.is_open()) {
		sock_.close();
		connected_ = false;
	}
}

bool AsioUDP::IsOpen() {
	return sock_.is_open();
}

AsioUDP::UDP::socket &AsioUDP::GetSocket() {
	return sock_;
}

bool AsioUDP::Connect(const string& ipPeer, const uint16_t port) {
	try {
		UDP::endpoint end(ip::address::from_string(ipPeer), port);
		sock_.connect(end);
		connected_ = true;
		start_read();
	}
	catch(error_code &ec) {
		return false;
	}
	return true;
}

const char *AsioUDP::Read(char *buff, int &n) {
	MtxLck lck(mtx_read_);
	if ((n = byte_read_)) memcpy(buff, buf_read_.get(), n);
	return n == 0 ? NULL : buff;
}

const char* AsioUDP::BlockRead(char *buff, int& n, const int millisec) {
	MtxLck lck(mtx_read_);
	boost::posix_time::milliseconds t(millisec);

	block_reading_ = true;
	cvread_.timed_wait(lck, t);
	if ((n = byte_read_)) memcpy(buff, buf_read_.get(), n);
	return n == 0 ? NULL : buff;
}

void AsioUDP::Write(const void *data, const int n) {
	MtxLck lck(mtx_write_);
	if (connected_) {
		sock_.async_send(buffer(data, n),
				boost::bind(&AsioUDP::handle_write, this,
						placeholders::error, placeholders::bytes_transferred));
	}
	else {
		sock_.async_send_to(buffer(data, n), remote_,
				boost::bind(&AsioUDP::handle_write, this,
						placeholders::error, placeholders::bytes_transferred));
	}
}

void AsioUDP::WriteTo(const string& ipPeer, const uint16_t portPeer, const void *data, int n) {
	MtxLck lck(mtx_write_);
	UDP::endpoint end(ip::address::from_string(ipPeer), portPeer);
	sock_.async_send_to(buffer(data, n), end,
			boost::bind(&AsioUDP::handle_write, this,
					placeholders::error, placeholders::bytes_transferred));
}

void AsioUDP::RegisterConnect(const CBSlot &slot) {
	cbconn_.connect(slot);
}

void AsioUDP::RegisterRead(const CBSlot &slot) {
	cbread_.connect(slot);
}

void AsioUDP::RegisterWrite(const CBSlot &slot) {
	cbwrite_.connect(slot);
}

void AsioUDP::start_read() {
	if (connected_) {
		sock_.async_receive(buffer(buf_read_.get(), UDP_PACK_SIZE),
				boost::bind(&AsioUDP::handle_read, this,
						placeholders::error, placeholders::bytes_transferred));
	}
	else {
		sock_.async_receive_from(buffer(buf_read_.get(), UDP_PACK_SIZE), remote_,
				boost::bind(&AsioUDP::handle_read, this,
						placeholders::error, placeholders::bytes_transferred));
	}
}

void AsioUDP::handle_read(const error_code& ec, const int n) {
	if (!ec || ec == error::message_size) {
		byte_read_ = n;
		if (block_reading_) cvread_.notify_one();
		else cbread_(shared_from_this(), ec);
		start_read();
	}
}

void AsioUDP::handle_write(const error_code& ec, const int n) {
	cbwrite_(shared_from_this(), ec);
}
