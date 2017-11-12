/*
 * @file SerialComm.cpp 定义文件, 基于boost::asio实现串口异步操作
 * @version 0.1
 * @date 2017-10-10
 */
#include <boost/make_shared.hpp>
#include "SerialComm.h"

using namespace boost::asio;

SerialPtr make_serial() {
	return boost::make_shared<SerialComm>();
}

SerialComm::SerialComm()
	: port_(keep_.get_service()) {
	bufrcv_.reset(new char[SERIAL_BUFF_SIZE]);
	crcrcv_.set_capacity(SERIAL_BUFF_SIZE * 10);
	crcsnd_.set_capacity(SERIAL_BUFF_SIZE * 10);
}

SerialComm::~SerialComm() {
	Close();
}

bool SerialComm::Open(const string& portname, const int baud_rate) {
	if (port_.is_open()) return true;
	error_code ec;
	port_.open(portname, ec);
	if (!ec) {
		port_.set_option(serial_port::baud_rate(baud_rate));
		port_.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
		port_.set_option(serial_port::parity(serial_port::parity::none));
		port_.set_option(serial_port::character_size(8));
		start_read();
	}
	else errmsg_ = ec.message();

	return !ec;
}

void SerialComm::Close() {
	if (port_.is_open()) {
		error_code ec;
		port_.close(ec);
	}
}

bool SerialComm::IsOpen() {
	return port_.is_open();
}

int SerialComm::Lookup(const char* flag, const int len, const int from) {
	if (!flag || len <= 0 || from < 0) return -1;

	mutex_lock lck(mtxrcv_);
	int n(crcrcv_.size() - len), pos, i, j;
	for (pos = from, i = -1; i != len && pos <= n; ++pos) {
		for (i = 0, j = pos; i < len && flag[i] == crcrcv_[j]; ++i, ++j);
	}
	return (i == len) ? (pos - 1) : -1;
}

const char* SerialComm::GetErrdesc() {
	return errmsg_.c_str();
}

int SerialComm::Write(const char* buff, const int len) {
	if (!buff || len <= 0 || !port_.is_open()) return 0;

	mutex_lock lck(mtxsnd_);
	int n0(crcsnd_.size()), n(crcsnd_.capacity() - n0), i;

	if (n > len) n = len;
	for (i = 0; i < n; ++i) crcsnd_.push_back(buff[i]);
	if (!n0) start_write();
	return n;
}

int SerialComm::Read(char* buff, const int len, const int from) {
	if (!buff || len <= 0 || from < 0) return 0;

	mutex_lock lck(mtxrcv_);
	int n0(crcrcv_.size()), n1(from + len), i(0), j(from);

	if (n0 > n1) n0 = n1;
	for (; j < n0; ++i, ++j) buff[i] = crcrcv_[j];
	if (i) crcrcv_.erase_begin(n0);
	return i;
}

void SerialComm::RegisterRead(const CBSlot& slot) {
	mutex_lock lck(mtxrcv_);
	if (!cbrcv_.empty()) cbrcv_.disconnect_all_slots();
	cbrcv_.connect(slot);
}

void SerialComm::RegisterWrite(const CBSlot& slot) {
	mutex_lock lck(mtxsnd_);
	if (!cbsnd_.empty()) cbsnd_.disconnect_all_slots();
	cbsnd_.connect(slot);
}

void SerialComm::handle_read(const error_code& ec, int n) {
	if (!ec) {
		mutex_lock lock(mtxrcv_);
		for(int i = 0; i < n; ++i) crcrcv_.push_back(bufrcv_[i]);
	}
	else errmsg_ = ec.message();
	if (!cbrcv_.empty()) cbrcv_((long) this, ec.value());
	if (!ec) start_read();
}

void SerialComm::handle_write(const error_code& ec, int n) {
	if (!ec) {
		mutex_lock lock(mtxsnd_);
		crcsnd_.erase_begin(n);
	}
	else errmsg_ = ec.message();
	if (!cbsnd_.empty()) cbsnd_((long) this, ec.value());
	if (!ec) start_write();
}

void SerialComm::start_read() {
	if (port_.is_open()) {
		port_.async_read_some(buffer(bufrcv_.get(), SERIAL_BUFF_SIZE),
				boost::bind(&SerialComm::handle_read, this,
						placeholders::error, placeholders::bytes_transferred));
	}
}

void SerialComm::start_write() {
	int n(crcsnd_.size());

	if (n) {
		port_.async_write_some(boost::asio::buffer(crcsnd_.linearize(), n),
				boost::bind(&SerialComm::handle_write, this,
						placeholders::error, placeholders::bytes_transferred));
	}
}
