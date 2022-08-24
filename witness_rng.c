>	witness64_d3d11.exe!shift_map(Entity_Machine_Panel * panel=0x0000000037d74a68, int n=6, int m=7, bool hflip=false, bool vflip=false, int cylinder_sides=36, int * vertex_map=0x000000004c89a540, int hoffset=0)	C++


m_prime = 0x7FFFFFFF; // 2^31 - 1, a mersenne prime
/* 0x1402AFE10 */ static int advance_rng() {
  if (_currentRng == 0) {
    _currentRng = 111111;
  }
  // This first operation is relatively low entropy, since the multiplicand is close to -2^31 (-2'147'483'648)
  // Thus, this operation is approximately (int32_t)((float)_currentRng / -2.053f);
  int nextRng = (int)(_currentRng * -2092037281L / 0x100000000L);
  nextRng = (_currentRng + nextRng) / 0x10000;
  // This is also kind of weird. It takes a different action if nextRng is even or odd.
  // This operation is also expressible as (nextRng % 2 == 0 : -nextRng ? 2^31 - nextRng)
  nextRng = -nextRng * m_prime;
  nextRng = nextRng + _currentRng * 16807; // 16807 == 7^5
  if (nextRng <= 0) { // Rarely true in practice
    nextRng = nextRng + m_prime; // Makes the result positive, even though the output is unsigned.
  }
  _currentRng = nextRng;
  return nextRng;
}

// Things that call and increment RNG (as an integer): 0x1402AFE10 / Random_Generator::get
00000001400C0AE7 // judge_pattern_flood_fill (doesn't seem to fire in practice)
00000001400C45D7 // shuffle_integers -> reroll_lotus_eater_stuff, update_bird_interrupt
00000001400CA92C // finish_speed_clock
00000001400CF031 // cut_random_edges
0000000140152947 // shuffle<int>(Auto_Array<int>)
0000000140152956 // shuffle<int>(Auto_Array<int>)
0000000140152BBA // get_empty_decoration_slot
0000000140152C17 // get_empty_dot_spot
0000000140154B83 // Entity_Machine_Panel::init_pattern_data_khatz (Left Peekaboo)
00000001401563BD // make_a_shaper
00000001401563E9 // make_a_shaper
0000000140156418 // make_a_shaper
0000000140156653 // add_exactly_this_many_bisection_dots
0000000140156966 // do_lotus_eights (doesn't seem to fire in practice)
0000000140156BC3 // do_lotus_minutes
0000000140156ED4 // do_lotus_tenths
0000000140157141 // Entity_Machine_Panel::init_pattern_data_lotus
00000001401578D2 // get_lotus_clock_shaper
0000000140157933 // get_lotus_clock_shaper
0000000140157957 // get_lotus_clock_shaper
0000000140157998 // get_lotus_clock_shaper
00000001401579BC // get_lotus_clock_shaper
0000000140157E67 // triangles
0000000140157E8F // triangles
0000000140157FDB // maze 3
000000014015800A // maze 3
000000014015812A // central pillar
0000000140158753 // dots pillar
0000000140158766 // dots pillar
0000000140158883 // pillars
000000014015888F // pillars
00000001401588B0 // pillars
00000001401588E3 // pillars
00000001401588FE // pillars
0000000140158A11 // pillars
0000000140158A21 // pillars
0000000140158DFB // random_shaper
0000000140158E18 // random_shaper
00000001401792B7 // position_decoy (note: decoy == triangle puzzle)
0000000140179618 // Entity_Record_Player::reroll_lotus_eater_stuff
000000014017965F // Entity_Record_Player::reroll_lotus_eater_stuff
00000001401B12F5 // get_non_repeating_index_zero_based -> (unused)
00000001401B1720 // play_footstep_sound
0000000140224406 // acquire_sound_from_name -> play_sound (metal_stress, so I assume more bunker stuff)
0000000140224DEB // do_player_sounds
0000000140225F63 // play_sound
000000014025F3AF // get_birdcall2_info
0000000140262D09 // update_creaky_bunker_background
00000001402DE7A3 // Priority_Queue::make_node -> Priority_Queue::add -> Mesh_Reducer stuff

// Things that call and increment RNG (as a float): 0x1402AFF40 / Random_Generator::get_within_range
00000001400B4C0F // do_end1_tv_intensity_proc -> Entity_Light::update
00000001400B52B9 // Entity_Light::update (spec_ruin)
00000001400B53B6 // Entity_Light::update (spec_ruin_after_laser)
00000001400B9127 // do_success_side_effects (red island control panel)
00000001400BF90F // do_success_side_effects (multipanel)
0000000140164FFC // get_direction -> Entity_Particle_Source::init_particle
0000000140165022 // get_direction -> Entity_Particle_Source::init_particle
00000001401657D9 // get_unit_vector -> Entity_Particle_Source::init_particle, update_noise_table
00000001401657EB // get_unit_vector -> Entity_Particle_Source::init_particle, update_noise_table
00000001401657FE // get_unit_vector -> Entity_Particle_Source::init_particle, update_noise_table
0000000140165FB9 // init_noise_table -> Particle_Manager::Particle_Manager()
0000000140165FCD // init_noise_table -> Particle_Manager::Particle_Manager()
0000000140165FE1 // init_noise_table -> Particle_Manager::Particle_Manager()
0000000140166148 // Entity_Particle_Source::init_particle
000000014016616E // Entity_Particle_Source::init_particle
000000014016619D // Entity_Particle_Source::init_particle
000000014016622C // Entity_Particle_Source::init_particle
0000000140166470 // Entity_Particle_Source::init_particle
000000014016649D // Entity_Particle_Source::init_particle
00000001401689E5 // Entity_Particle_Source::spawn_on_circle
000000014016A514 // Entity_Particle_Source::spawn_on_path
00000001401756DA // Entity_Pressure_Plate::reset
0000000140175706 // Entity_Pressure_Plate::reset
0000000140175A8C // reset_pressure_plates -> do_success_side_effects (pressure plate reset)
0000000140175AB8 // reset_pressure_plates -> do_success_side_effects (pressure plate reset)
0000000140175EAA // Entity_Pressure_Plate::stepped_on
0000000140175ED6 // Entity_Pressure_Plate::stepped_on
000000014017642A // Entity_Pressure_Plate::update
0000000140176456 // Entity_Pressure_Plate::update
0000000140176545 // Entity_Pressure_Plate::update
0000000140176571 // Entity_Pressure_Plate::update
00000001401765EE // Entity_Pressure_Plate::update
000000014017661A // Entity_Pressure_Plate::update
0000000140198CE6 // do_env_pattern_gesture_abort_finish_indications -> do_gesture_abort_finish_indications -> do_post_trace_actions -> Gesture_Handler::update
000000014020F08C // make_point_sound -> Gesture, test_ray_against_points -> test_initial_click_against_points
0000000140224D2A // do_player_sounds -> simulate_guy -> update_camera
0000000140224D90 // do_player_sounds
0000000140224DB3 // do_player_sounds
0000000140224E24 // play_sound -> (unused)
0000000140224E8D // play_sound
0000000140225F1B // play_sound
0000000140225F48 // play_sound
000000014025DE7C // bird_randomize_volume
000000014025DEA8 // bird_randomize_volume
000000014025F37C // get_birdcall2_info
000000014025F39A // get_birdcall2_info
000000014025F497 // get_birdcall2_info
000000014025F4C0 // get_birdcall2_info
000000014025F4E9 // get_birdcall2_info
000000014025F975 // get_birdcall_dots_info
000000014025F9C0 // get_birdcall_dots_info
00000001402619FF // roll_two_numbers -> bird stuff
0000000140261A10 // roll_two_numbers -> bird stuff
0000000140261A40 // roll_two_numbers -> bird stuff
0000000140261A51 // roll_two_numbers -> bird stuff
0000000140262084 // update_bird1
00000001402622BB // update_bird2
000000014026250B // update_bird_dots
00000001402627B0 // update_bird_interrupt
0000000140262866 // update_bird_interrupt
00000001402628DB // update_bird_interrupt
00000001402628FF // update_bird_interrupt
0000000140262967 // update_bird_interrupt
0000000140262DCC // update_creaky_bunker_background
0000000140262DEB // update_creaky_bunker_background
0000000140262F53 // update_creaky_bunker_bass
0000000140262FD9 // update_creaky_bunker_bass
0000000140263065 // update_creaky_bunker_bass
00000001402630F6 // update_creaky_bunker_bass
000000014026314B // update_creaky_bunker_bass
000000014026324D // update_creaky_bunker_treble
00000001402632E1 // update_creaky_bunker_treble
0000000140263381 // update_creaky_bunker_treble
000000014026341E // update_creaky_bunker_treble
000000014026345C // update_creaky_bunker_treble
