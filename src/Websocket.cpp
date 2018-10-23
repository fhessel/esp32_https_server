/*
 * Websocket.cpp
 *
 *  Created on: 12.09.2018
 *      Author: springob
 */
#include <sstream>
#include "Websocket.hpp"
#include "HTTPConnection.hpp"

namespace httpsserver {

/**
 * @brief Dump the content of the WebSocket frame for debugging.
 * @param [in] frame The frame to dump.
 */
static void dumpFrame(Frame frame) {
	std::ostringstream oss;
	oss << "Fin: " << (int)frame.fin << ", OpCode: " << (int)frame.opCode;
	switch(frame.opCode) {
		case OPCODE_BINARY: {
			oss << " BINARY";
			break;
		}
		case OPCODE_CONTINUE: {
			oss << " CONTINUE";
			break;
		}
		case OPCODE_CLOSE: {
			oss << " CLOSE";
			break;
		}
		case OPCODE_PING: {
			oss << " PING";
			break;
		}
		case OPCODE_PONG: {
			oss << " PONG";
			break;
		}
		case OPCODE_TEXT: {
			oss << " TEXT";
			break;
		}
		default: {
			oss << " Unknown";
			break;
		}
	}
	oss << ", Mask: " << (int)frame.mask << ", len: " << (int)frame.len;
	std::string s = "[Frm] " + oss.str();
	HTTPS_DLOG(s.c_str());
} // dumpFrame


WebsocketHandler::~WebsocketHandler() {
} // ~WebSocketHandler()


/**
* @brief The default onClose handler.
* If no over-riding handler is provided for the "close" event, this method is called.
*/
void WebsocketHandler::onClose() {
	HTTPS_DLOG("[   ] WebsocketHandler close()");
} // onClose

/**
* @brief The default onData handler.
* If no over-riding handler is provided for the "message" event, this method is called.
* A particularly useful pattern for using onMessage is:
* ```
* std::stringstream buffer;
* buffer << pWebSocketInputRecordStreambuf;
* ```
* This will read the whole message into the string stream.
*/
void WebsocketHandler::onMessage(WebsocketInputStreambuf* pWebsocketInputStreambuf, Websocket *pWebSocket) {
	HTTPS_DLOG("[   ] WebsocketHandler onMessage()");
} // onData


/**
* @brief The default onError handler.
* If no over-riding handler is provided for the "error" event, this method is called.
*/
void WebsocketHandler::onError(std::string error) {
	HTTPS_DLOG("[   ] WebsocketHandler onError()");
} // onError


Websocket::Websocket(HTTPConnection *con) {
	_con = con;
	_receivedClose     = false;
	_sentClose         = false;
	_wsHandler 			= nullptr;	
}

Websocket::~Websocket() {
	// TODO Auto-generated destructor stub
}

int Websocket::read() {
	Frame frame;
	_con->updateBuffer();
	int length = _con->readBuffer((uint8_t*)&frame, sizeof(frame));
	HTTPS_DLOGHEX("[   ] read bytes:", length);
	if(length == 0) 
		return 0;
	else if (length != sizeof(frame)) {
		HTTPS_DLOG("[ERR] Websocket read error");
		//_con->closeConnection();
		return -1;
	}
	dumpFrame(frame);

	// The following section parses the WebSocket frame.
	uint32_t payloadLen = 0;
	uint8_t  mask[4];
	if (frame.len < 126) {
		payloadLen = frame.len;
	} else if (frame.len == 126) {
		uint16_t tempLen;
		_con->readBuffer((uint8_t*)&tempLen, sizeof(tempLen));
		payloadLen = ntohs(tempLen);
	} else if (frame.len == 127) {
		uint64_t tempLen;
		_con->readBuffer((uint8_t*)&tempLen, sizeof(tempLen));
		payloadLen = ntohl((uint32_t)tempLen);
	}
	if (frame.mask == 1) {
		_con->readBuffer(mask, sizeof(mask));
	}

	if (payloadLen == 0) {
		HTTPS_DLOG("[WRN] Web socket payload is not present");
	} else {
		HTTPS_DLOGHEX("[   ] Web socket payload, length=%d:", payloadLen);
	}

	switch(frame.opCode) {
		case OPCODE_TEXT:
		case OPCODE_BINARY: {
			if (_con->_wsHandler != nullptr) {
				WebsocketInputStreambuf streambuf(_con, payloadLen, frame.mask==1?mask:nullptr);
				_con->_wsHandler->onMessage(&streambuf, this);
				streambuf.discard();
			}
			break;
		}

		case OPCODE_CLOSE: {  // If the WebSocket operation code is close then we are closing the connection.
			_receivedClose = true;
			if (_con->_wsHandler != nullptr) { // If we have a handler, invoke the onClose method upon it.
				_con->_wsHandler->onClose();
			}
			//close();                // Close the websocket.
			return -1;
			break;
		}

		case OPCODE_CONTINUE: {
			break;
		}

		case OPCODE_PING: {
			break;
		}

		case OPCODE_PONG: {
			break;
		}

		default: {
				HTTPS_DLOGHEX("[   ] WebSocketReader: Unknown opcode: ", frame.opCode);
			break;
		}
	} // Switch opCode
	return 0;
}  // Websocket::read

/**
 * @brief Close the Web socket
 * @param [in] status The code passed in the close request.
 * @param [in] message A clarification message on the close request.
 */
void Websocket::close(uint16_t status, std::string message) {
	HTTPS_DLOG("[   ] >> Websocket close()");

	_sentClose = true;              // Flag that we have sent a close request.

	Frame frame;                     // Build the web socket frame indicating a close request.
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = OPCODE_CLOSE;
	frame.mask   = 0;
	frame.len    = message.length() + 2;
	int rc = _con->writeBuffer((uint8_t *)&frame, sizeof(frame));

	if (rc > 0) {
		rc = _con->writeBuffer((byte *) &status, 2);
	}

	if (rc > 0) {
		_con->writeBuffer((byte *) message.data(), message.length());
	}
} // Websocket::close

/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void Websocket::send(std::string data, uint8_t sendType) {
	HTTPS_DLOGHEX(">> Websocket.send: Length: ", data.length());
	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = sendType==SEND_TYPE_TEXT?OPCODE_TEXT:OPCODE_BINARY;
	frame.mask   = 0;
	if (data.length() < 126) {
		frame.len = data.length();
		_con->writeBuffer((uint8_t *)&frame, sizeof(frame));
	} else {
		frame.len = 126;
		_con->writeBuffer((uint8_t *)&frame, sizeof(frame));
		uint16_t net_len = htons((uint16_t)data.length());
		_con->writeBuffer((uint8_t *)&net_len, sizeof(uint16_t));  // Convert to network byte order from host byte order
	}
	_con->writeBuffer((uint8_t*)data.data(), data.length());
	HTTPS_DLOG("[   ] << Websocket.send");
} // Websocket::send


/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void Websocket::send(uint8_t* data, uint16_t length, uint8_t sendType) {
	HTTPS_DLOGHEX("[   ] >> Websocket.send: Length: ", length);
	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = sendType==SEND_TYPE_TEXT?OPCODE_TEXT:OPCODE_BINARY;
	frame.mask   = 0;
	if (length < 126) {
		frame.len = length;
		_con->writeBuffer((uint8_t *)&frame, sizeof(frame));
	} else {
		frame.len = 126;
		_con->writeBuffer((uint8_t *)&frame, sizeof(frame));
		uint16_t net_len = htons(length);
		_con->writeBuffer((uint8_t *) net_len, sizeof(uint16_t));  // Convert to network byte order from host byte order
	}
	_con->writeBuffer(data, length);
	HTTPS_DLOG("[   ] << Websocket.send");
}  // Websocket::send



/**
 * @brief Create a Web Socket input record streambuf
 * @param [in] socket The socket we will be reading from.
 * @param [in] dataLength The size of a record.
 * @param [in] bufferSize The size of the buffer we wish to allocate to hold data.
 */
WebsocketInputStreambuf::WebsocketInputStreambuf(
	HTTPConnection   *socket,
	size_t   dataLength,
	uint8_t *pMask,
	size_t   bufferSize) {
	_con     = socket;    // The socket we will be reading from
	_dataLength = dataLength; // The size of the record we wish to read.
	_pMask      = pMask;
	_bufferSize = bufferSize; // The size of the buffer used to hold data
	_sizeRead   = 0;          // The size of data read from the socket
	_buffer = new char[bufferSize]; // Create the buffer used to hold the data read from the socket.

	setg(_buffer, _buffer, _buffer); // Set the initial get buffer pointers to no data.
} // WebSocketInputStreambuf

/**
 * @brief Destructor
 */
WebsocketInputStreambuf::~WebsocketInputStreambuf() {
	delete[] _buffer;
	discard();
} // ~WebSocketInputRecordStreambuf

/**
 * @brief Discard data for the record that has not yet been read.
 *
 * We are working on a logical fixed length record in a socket stream.  This means that we know in advance
 * how big the record should be.  If we have read some data from the stream and no longer wish to consume
 * any further, we have to discard the remaining bytes in the stream before we can get to process the
 * next record.  This function discards the remainder of the data.
 *
 * For example, if our record size is 1000 bytes and we have read 700 bytes and determine that we no
 * longer need to continue, we can't just stop.  There are still 300 bytes in the socket stream that
 * need to be consumed/discarded before we can move on to the next record.
 */
void WebsocketInputStreambuf::discard() {
	uint8_t byte;
	HTTPS_DLOGINT("[   ] WebsocketInputStreambuf >> discard: Discarding bytes: ", _dataLength - _sizeRead);
	while(_sizeRead < _dataLength) {
		_con->readBuffer(&byte, 1);
		_sizeRead++;
	}
	HTTPS_DLOG("[   ] WebsocketInputStreambuf << discard");
} // WebsocketInputStreambuf::discard


/**
 * @brief Get the size of the expected record.
 * @return The size of the expected record.
 */
size_t WebsocketInputStreambuf::getRecordSize() {
	return _dataLength;
} // WebsocketInputStreambuf::getRecordSize

/**
 * @brief Handle the request to read data from the stream but we need more data from the source.
 *
 */
WebsocketInputStreambuf::int_type WebsocketInputStreambuf::underflow() {
	HTTPS_DLOG("[   ] WebSocketInputStreambuf >> underflow");

	// If we have already read as many bytes as our record definition says we should read
	// then don't attempt to ready any further.
	if (_sizeRead >= getRecordSize()) {
		HTTPS_DLOG("[   ] WebSocketInputStreambuf << underflow: Already read maximum");
		return EOF;
	}

	// We wish to refill the buffer.  We want to read data from the socket.  We want to read either
	// the size of the buffer to fill it or the maximum number of bytes remaining to be read.
	// We will choose which ever is smaller as the number of bytes to read into the buffer.
	int remainingBytes = getRecordSize()-_sizeRead;
	size_t sizeToRead;
	if (remainingBytes < _bufferSize) {
		sizeToRead = remainingBytes;
	} else {
		sizeToRead = _bufferSize;
	}

	HTTPS_DLOGINT("[   ] WebSocketInputRecordStreambuf - getting next buffer of data; size request: ", sizeToRead);
	int bytesRead = _con->readBuffer((uint8_t*)_buffer, sizeToRead);
	if (bytesRead == 0) {
		HTTPS_DLOG("[   ] WebSocketInputRecordStreambuf << underflow: Read 0 bytes");
		return EOF;
	}

	// If the WebSocket frame shows that we have a mask bit set then we have to unmask the data.
	if (_pMask != nullptr) {
		for (int i=0; i<bytesRead; i++) {
			_buffer[i] = _buffer[i] ^ _pMask[(_sizeRead+i)%4];
		}
	}

	_sizeRead += bytesRead;  // Increase the count of number of bytes actually read from the source.

	setg(_buffer, _buffer, _buffer + bytesRead); // Change the buffer pointers to reflect the new data read.
	HTTPS_DLOGINT("[   ] WebSocketInputRecordStreambuf << underflow - got more bytes: ", bytesRead);
	return traits_type::to_int_type(*gptr());
} // underflow

} /* namespace httpsserver */
