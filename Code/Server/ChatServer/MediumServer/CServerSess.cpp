﻿#include "CClientSess.h"
namespace ChatServer
{
std::shared_ptr<spdlog::logger> CServerSess::ms_loger;

/**
 * @brief 从socket的读取数据的函数
 * 
 */
void CServerSess::do_read()
{
	auto self = shared_from_this();
	m_socket.async_read_some(
		asio::buffer(m_recvbuf + m_recvpos, max_length - m_recvpos),
			[this, self](std::error_code ec, std::size_t length) {
			if (length > sizeof(TransBaseMsg_t))
			{

				TransBaseMsg_t msg(m_recvbuf);
				auto curlen = m_recvpos + length;
				while (curlen >= sizeof(Header) && curlen >= msg.GetSize())
				{
					handle_message(&msg);
					curlen -= msg.GetSize();
					memmove(m_recvbuf, m_recvbuf + msg.GetSize(), curlen);
				}
				m_recvpos = (uint32_t)curlen;
				if (m_recvpos < max_length && !ec)
				{
					do_read();
				}
				else
				{
					CloseSocket();
				}
			}
		});
}

/**
 * @brief 当do_read函数接收到一个完整消息的时候，调用此函数，在此函数中完成消息类型的判断和消息分发
 * 
 * @param hdr 需要被处理的消息
 */
void CServerSess::handle_message(const TransBaseMsg_t *hdr)
{
	if (m_server)
	{
		m_server->DispatchRecvTcpMsg(shared_from_this(), hdr);
	}
}


/**
 * @brief 处理心跳回复消息
 * 
 * @param rspMsg 心跳回复消息
 */
void CServerSess::handleKeepAliveRsp(const KeepAliveRspMsg &rspMsg)
{
	LOG_INFO(ms_loger, "KeepAliveRsp:{} [ {} {} ] ", rspMsg.ToString(), __FILENAME__, __LINE__);
}



/**
 * @brief 关闭socket
 * 
 */
void CServerSess::CloseSocket() {
	m_socket.close();
	m_bConnect = false;
	LOG_INFO(ms_loger, "{} Leave Server [ {} {}]", UserId(), __FILENAME__, __LINE__);
	if (m_server) {
		m_server->OnSessClose(shared_from_this());
	}
}

} // namespace MediumServer