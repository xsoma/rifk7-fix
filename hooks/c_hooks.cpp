
#include "c_hooks.h"
#include "c_wnd_proc.h"
#include "idirect3ddevice9.h"
#include "c_client_mode_shared.h"
#include "c_surface_.h"
#include "c_events.h"
#include "c_model_render_.h"
#include "c_client.h"
#include "c_game_event_manager_.h"
#include "c_engine_client.h"
#include "c_base_animating_.h"
#include "c_cl_csm_shadows.h"
#include "c_net_channel_.h"
#include "c_cs_player_.h"
#include "dbghelp.h"
#include <thread>
#include "c_cl_camera_height_restriction_debug.h"
#include "c_view_render_.h"
#include "c_prediction_.h"
#include "c_client_state_.h"
#include "c_mdl_cache.h"
#include "c_render_view_.h"
#include "c_engine_sound_.h"
#include "c_sv_cheats.h"
#include "c_panel_.h"
#include "c_smoke_grenade_projectile_.h"

using namespace std::chrono_literals;

void c_hooks::run()
{
	c_client_mode_shared::hook();
	c_wnd_proc::hook();
	c_cs_player_::hook();
	c_surface_::hook();
	c_events::hook();
	c_render_view_::hook();
	c_model_render_::hook();
	c_client::hook();
	c_game_event_manager_::hook();
	c_base_animating_::hook();
	c_engine_client_::hook();
	c_cl_csm_shadows::hook();
	c_cl_camera_height_restriction_debug::hook();
	c_view_render_::hook();
	c_prediction_::hook();
	c_client_state_::hook();
	c_mdl_cache::hook();
	c_engine_sound_::hook();
	c_sv_cheats::hook();
	c_panel_::hook();
	c_smoke_grenade_projectile_::hook();
}
