#pragma once

#include "../includes.h"
#include "../sdk/c_client_state.h"
#include <deque>

struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int OutSequenceNr, int outrel, int seqnr, float time)
	{
		inreliablestate = instate;
		m_nOutSequenceNr = OutSequenceNr;
		m_nOutReliableState = outrel;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int m_nOutReliableState;
	int m_nOutSequenceNr;
	int sequencenr;
	float curtime;
};
class c_net_channel_
{

	typedef void(__thiscall* process_packet_t)(c_net_channel*, void*, bool);
	typedef bool(__thiscall* send_netmsg_t)(c_net_channel*, c_net_msg*, bool, bool);
	typedef int(__thiscall* send_datagram_t)(c_net_channel*, void*);

public:
	static void hook();

	static void apply_to_net_chan(c_net_channel* channel);

	inline static std::deque<CIncomingSequence>sequences;
	inline static void UpdateIncomingSequences(c_net_channel* channel);
	inline static std::unique_ptr<c_hook<uint32_t>> hk;
	inline static process_packet_t _process_packet;
	inline static send_netmsg_t _send_netmsg;
	inline static send_datagram_t _send_datagram;
private:
	static void __fastcall process_packet(c_net_channel* channel, uint32_t, void* packet, bool header);
	static bool __fastcall send_netmsg(c_net_channel* channel, uint32_t, c_net_msg* msg, bool reliable, bool voice);
	static int __fastcall send_datagram(c_net_channel* channel, uint32_t, void* buffer);
};

#define cnetchannel c_net_channel_::instance()