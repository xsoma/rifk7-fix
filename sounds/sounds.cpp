#include "sounds.h"
#include "../security/string_obfuscation.h"
#include "../BASS/API.h"
#include "../utils/c_config.h"
#include "../c_rifk.h"
#include <thread>
#include "../menu/c_menu.h"

using namespace std::chrono_literals;

void playback_loop()
{
	while (!memory::get_module_handle(fnv1a("serverbrowser.dll")))
		std::this_thread::sleep_for(100ms);

	c_rifk::instance();

	BASS::bass_lib_handle = BASS::bass_lib.LoadFromMemory(bass_dll_image, sizeof(bass_dll_image));

	if (BASS_INIT_ONCE())
		BASS::bass_init = TRUE;

	static auto bass_needs_reinit = false;

	static std::pair<std::string, char> channels[] = {
		__(" "),
		__("http://chanson.hostingradio.ru:8041/chanson-uncensored128.mp3"),
		__("http://air2.radiorecord.ru:805/sd90_128"),
		__("http://streams.bigfm.de/bigfm-deutschrap-128-aac"),
		__("http://air2.radiorecord.ru:805/mini_128"),
		__("http://air2.radiorecord.ru:805/chil_128"),
		__("https://podrubasik.ru/radio/8000/radio.mp3"),
		__("https://www.internet-radio.com/servers/tools/playlistgenerator/?u=http://158.69.114.190:8042/listen.pls?sid=1&t=.m3u"),
		__("https://www.internet-radio.com/servers/tools/playlistgenerator/?u=http://198.100.145.185:8900/listen.pls?sid=1&t=.m3u"),
		__("http://icecast.vgtrk.cdnvideo.ru/vestifm_mp3_128kbps")
	};

	hotkeys->register_callback([](const uint32_t code) -> void
	{
		if (code != 0 && code == config.misc.radio_mute)// && !menu->is_open())
			radio_muted = !radio_muted;
	});



	while (true)
	{
		std::this_thread::sleep_for(100ms);
		const auto desired_channel = config.misc.radio_channel;

		if (BASS::bass_init && desired_channel)
		{
			static auto current_channel = 0;

			if (current_channel != desired_channel || bass_needs_reinit)
			{
				bass_needs_reinit = false;
				BASS_Start();
				_rt(channel, channels[desired_channel]);
				BASS_OPEN_STREAM(channel);
				current_channel = desired_channel;
			}
			
			BASS_SET_VOLUME(BASS::stream_handle, radio_muted ? 0.f : config.misc.radio_volume / 100.f);
			BASS_PLAY_STREAM();
		}
		else if (BASS::bass_init)
		{
			bass_needs_reinit = true;
			BASS_StreamFree(BASS::stream_handle);
		}
	}
}
