{"vehicle"
	{inflammation_time 20}
	{on spawn
		{add_view	"stop_signal"		"stop_signal"	"FXstop1"}
		{add_view	"stop_signal"		"stop_signal"	"FXstop2"}

		;// Light Bulb FX
		{add_view 	"headlight"			"lights"		"headlightl"}
		{add_view 	"headlight"			"lights"		"headlightr"}
		{add_view 	"headlight"			"lights"		"headlightc"}
		{add_view 	"headlight"			"lights"		"headlightl1"}
		{add_view 	"headlight"			"lights"		"headlightr1"}
		{add_view 	"headlight2"		"lights"		"armorlightl"}
		{add_view 	"headlight2"		"lights"		"armorlightr"}
		{add_view 	"headlight_dim"		"lights"		"dimlight"}

		;// Light
		{add_view 	"light_vehicle"		"lights"		"headlightl"}
		{add_view 	"light_vehicle"		"lights"		"headlightr"}
		{add_view 	"light_vehicle"		"lights"		"headlightc"}
		{add_view 	"light_vehicle_dim"	"lights"		"armorlightl"}
		{add_view 	"light_vehicle_dim"	"lights"		"armorlightr"}
		{add_view 	"light_vehicle_dim"	"lights"		"dimlight"}

		;// Mission helpers
		{add_view	"ammo" 				"ammo" "body"}


		{view hide "stop_signal"}
		{view hide "lights"}
		{view hide "highlight"}

		{call "init_gun"}
		{call "add_movement_sound"}
		{call "update_moveable"}
		{call "update_lights"}
		{call "engine_check"}
		{set "burn_fire" 0}
	}

	;/*=========== Hardcode Events Begin  ===========*/

	{on turn on 				;// applies to track chassis neutral turn
		{call "stop_on"}
		{call "exhaust_on"}
		{play_sound "move" 1}
		{stop_sound "engine_idle"}
	}

	{on turn off 				;// applies to track chassis neutral turn
		{call "exhaust_off"}
		{stop_sound "move"}
	}

	{on move on 				;// any vehicle moves forward or reverse
		{stop_sound "engine_idle"}
		{stop_sound "move_end"}
		{play_sound "move" 1}
		{call "exhaust_on"}
		{if not "animated"						;// Prevents animation from playing when "stop" animation is currently playing
			;{ani_stop "stop"}
			{ani_play "start" 1.0 callback}
			{set "animated" 1}
		}
		{view pause "exhaust2"}
		{if tagged "vehicle_detect_motion"		;// used for mission scripts
			{tags add "vehicle_in_motion"}		;// used for mission scripts
		}
	}

	{on move off 				;// any vehicle not moving forward or reverse
		{stop_sound "move"}
		{tags remove "brake_on"}
		{play_sound "move_end"}
		{call "exhaust_off"}
		{tags remove "vehicle_in_motion"}		;// used for mission scripts
	}

	{on accelerate on			;// any vehicle that is accelerating
		{call "exhaust_on"}
	}

	{on accelerate off			;// any vehicle that is decelerating, or maintaining current speed
		{call "exhaust_off"}
	}

	{on brake on				;// any vehicle applying brakes to slow down
		{call "stop_on"}
		{play_sound "brake"}
		{view show "stop_signal"}
		{view start "stop_signal"}
	}

	{on brake off				;// any vehicle releases brakes
		{stop_sound "brake"}
		{delay 0.5
			{view hide "stop_signal"}
		}
	}

	{on engine on				;// any vehicle moves forward, reverse or preforming neutral turn
		{if "engine_on"							;// checks that engine is on and not toggled off while moving
			{kill_delay "engine_stopped"}
			{kill_delay "engine_steam_stopped"}
			{stop_sound "engine_stop"}
			{play_sound "exhaust"}				;// exhaust/accelerate sound
			{play_sound "engine_full" 1}
			{view start "engine_steam"}			;// used for train steam fx, and vehicle engines in rain
			{if "hidden"
				{call "engine_idle"}
				{call "play_trace_engine"}		;// plays dust fx for aero vehicles
				{call "update_lights"}
				{set "hidden" 0}
			}
		}
	}

	{on engine off				;// any vehicle not moving at all
		{if not "engine_on"
		  {stop_sound "engine_full"}
			{play_sound "brake"}
			{view pause "exhaust"}
			{view pause "exhaust2"}
		}
		{if "engine_on"
			{play_sound "exhaust"}
			{delay 0.5 "engine_stopped"
				{if work "engine"
					{if work "body"
						{if "engine_on"
							{play_sound "engine_idle" 1}
							{stop_sound "engine_full"}
						}
					}
				}
			}
			{delay 0.5 "engine_steam_stopped"
				{if work "engine"
					{if work "body"
						{if "engine_on"
							{if work "mover"
								{view start "exhaust2"}
							}
						}
					}
				}
			}
		}
	}

	{on operatable on
		{call "repaired_engine_check"} 		;// calls check to restart engine after body repair and crew re-enters vehicle
		{call "check_water_level"}
	}

	{on operatable off
		{unlink_trailer}
		{kill_delay "emit1"}
		{kill_delay "emit2"}
		{kill_delay "emit3"}
		{kill_delay "emit4"}
	}

	{on break
		{if not tagged "exploded"
			{if component "body"
				{view stop "ammo"}
				{call "stat_break"}
				{call "emit_crew"}
				{call "engine stop view"}
				{call "deactivate_vehicle"}
			}
			{if component "engine"
				{call "engine stop view"}
				{talk "broken_engine"}
			}
			{if component "glass"
				{call "break_glass"}
			}
			{if not component multiple 		;// if the changed component is the "last" one. If only a single component changes or if it is the last component of multiple ones (like with {components destroy}, this will yield "true".
				{call "update_moveable"}
				{call "break_light"}
			}
		}
	}

	{on repair
		{call "update_moveable"}
		{call "repaired_engine_check"}
		{if component "body"
			{call "activate_vehicle"}
		}
	}

	{on contact sync
		{if volume "sticky"
			{if effector "grenade"
				{if not effector "contact"
					{call "grenade_stick"}
				}
			}
		}
		{if not impregnable
			{if effector_velocity 3
				{if effector "airborne"
					{component break}
				else effector "piece_big_stone"
					{component break}
				}
			}
		}
		{if effector "grenade fire"		;// molotov grenades
			{burn fx "smoke_molot_vehicle" time 20 fake}
			{start_sound "detonation/small/molotov_break"}
			{talk "burning"}
		}
		{if effector "burn_piece"		;// ic shell
			{burn fx "smoke_molot_vehicle" time 20 fake}
			{start_sound "detonation/small/molotov_break"}
			{talk "burning"}
		}
	}

	{on blast_wreaked sync
		{vehicle_health_damage_blast}
	}

	{on destroy
		{if component "body"
			{call "explosion"}
			{view stop "ammo"}
		else component "engine"
			{call "_burn"}
			{view stop "ammo"}

		}
	}

	;/*=========== Hardcode Events End  ===========*/

	;/*=========== Engine Code Begin  ===========*/

	{on "engine_toggle" 		;// UI button to toggle engine on/off
		{if operatable
			{if work "engine"
				{if work "body"
					{if not "engine_on"
						{if not "toggle_delay"
							{call "engine_on"}
						}
					else not "toggle_delay"
						{call "engine_off"}
					}
				}
			else
				{tags add "engine_turned_off"}	;// allows engine to be set to off state when damaged. Cancels auto engine start on repair
			}
		}
		{set "toggle_delay" 1}			;// stops accidental double click of UI button
		{delay 1 {set "toggle_delay" 0}}
	}
	{on "engine_check" 		;// turns engine on at spawn if crewed and not tagged engine off
		{set "engine_check" 1}
		{delay 0.1
			{if tagged "engine_turned_off"
				{chassis_work 0}
				{set "engine_on" 0}
			else
				{if operatable
					{if tagged "hidden" 	;// mission script that hides unit
						{set "hidden" 1}
					else
						{call "engine_idle"}
						{call "play_trace_engine"} 		;// plays dust fx for aero vehicles
						{call "update_lights"}
					}
				}
			}
			{set "engine_check" 0}
		}
	}
	{on "repaired_engine_check" 		;// turns engine on when repaired if crewed and not tagged engine off
		{if not "engine_check" 			;// stops call from happening at spawn
			{if operatable
				{if not tagged "engine_turned_off"
					{if not "engine_on"
						{call "engine_on"}
					}
				}
			}
		}
	}
	{on "engine_on" 			;// settings for engine when switched on
		{if work "body"
			{if work "engine"
				{tags remove "engine_turned_off"}
				{view show "stop_signal"}
				{view start "stop_signal"}
				{play_sound "engine_start"}
				{ani_play "engine_work" loop}
				{set "engine_on" 1}
				{call "update_lights"}
				{add_view	"exhaust_dark_big_startup"	"exhaust_start"	"FXsmoke1"}
				{add_view	"exhaust_dark_big_startup"	"exhaust_start"	"FXsmoke2"}
				{delay 1.5 "engine_startup"
					{view start "exhaust_start"}
					{delay 0.5
						{view pause "exhaust_start"}
					}
					{delay 1.25 "engine_startup2"
						{play_sound "exhaust"}
					}
					{delay 1.5 "engine_startup3"
						;{play_sound "exhaust"}
						{call "engine_idle"}
					}
				}
			}
		}
	}

	{on "engine_off" 						;// settings for engine when switched off
		{kill_delay "engine_startup"}		;// stops engine startup sequence
		{kill_delay "engine_startup2"}		;// stops engine startup sequence
		{kill_delay "engine_startup3"}		;// stops engine startup sequence
		{set "engine_on" 0}
		{call "update_lights"}
		{stop_sound "engine_idle"}
		{stop_sound "move"}
		{stop_sound "engine_full"}
		{stop_sound "engine_start"}
		{play_sound "engine_stop"}
		{tags add "engine_turned_off"}		;// tracks engine on/off status for mission save/load
		{chassis_work 0}
		{delay 1
			{view pause "exhaust2"}
			{ani_stop "engine_work"}
			{view hide "stop_signal"}
		}
	}

	{on "engine_idle"					;// settings for engine while idle
		{if work "engine"
			{if work "trackleft"
				{if work "trackright"
					{if not tagged "wheel_destroyed"		;// Checks if wheel has been torn off
						{chassis_work 1}
					}
				}
			}
			{set "engine_on" 1}
			{view pause "exhaust_start"}
			{stop_sound "engine_start"}
			{play_sound "engine_idle" 1}
			{ani_play "engine_work" loop}
			{view start "exhaust2"}
			{view hide "stop_signal"}
		}
	}

	{on "engine stop view"				;// called on engine and body break
		{set "engine_on" 0}
		{call "update_lights"}
		{kill_delay "engine_startup"}		;// stops engine startup sequence
		{kill_delay "engine_startup2"}		;// stops engine startup sequence
		{kill_delay "engine_startup3"}		;// stops engine startup sequence
		{view pause "exhaust"}
		{stop_sound "engine_idle"}
		{stop_sound "engine_full"}
		{stop_sound "engine_start"}
		{ani_stop "engine_work"}
		{view pause "exhaust2"}
	}

	{on "engine fullstop"					;// called on explosion
		{set "engine_on" 0}
		{kill_delay "engine_startup"}		;// stops engine startup sequence
		{kill_delay "engine_startup2"}		;// stops engine startup sequence
		{kill_delay "engine_startup3"}		;// stops engine startup sequence
		{view pause "exhaust"}
		{ani_stop "engine_work"}
		{delay 0 "engine_stopped"
			{stop_sound "engine_idle"}
			{stop_sound "engine_start"}
			{stop_sound "engine_stop"}
		}
		{delay 0 "engine_steam_stopped"
			{view pause "exhaust2"}
		}
	}

	{on "update_moveable"
		{if work "engine"
			{call "check_work_mover"}
		else
			{chassis_work 0}
		}
	}

	{on "check_work_mover"
		{if work "mover"
			{if not tagged "wheel_destroyed"		;// Checks if wheel has been torn off
				{if not tagged "engine_turned_off"	;// Checks that player didn't turn off engine before repairs are made
					{chassis_work 1}
				}
			}
		else
			{chassis_work 0}
		}
	}

	{on "stop_on"
		{if work "body"						;// Prevents animations from reseting body break animations if vehicle is moving on break
			{if not "animated"				;// Prevents animation from playing when "start" animation is currently playing
				;{ani_stop "start"}
				{ani_play "stop" 1.0 callback}
				{set "animated" 1}
			}
		}
	}

	{on "exhaust_on"
		{if "engine_on"						;// Prevents sounds from playing if called after engine is off/broken
			{view start "exhaust"}
			{play_sound "exhaust"}
		}
	}

	{on "exhaust_off"
;//		{stop_sound "exhaust"}
		{view pause "exhaust"}
		{view start "exhaust2"}				;// idle exhaust
	}

	;/*=========== Engine Code End  ===========*/

	;/*=========== Light FX Begin  ===========*/

	{on "lights_on"
		{view start "lights"}
		{view show "lights"}
	}
	{on "lights_off"
		{view stop "lights"}
		{view hide "lights"}
	}
	{on "update_lights"
		{if not sunlight					;// checks if night time to show headlight fx
			{if "engine_on"
				{call "lights_on"}
			else
				{call "lights_off"}
			}
		else
			{call "lights_off"}
		}
		{if rain							;// checks if raining to play windshield wipers
			{if "engine_on"
				{ani_play "wipers" loop resume}
			else
				{ani_stop "wipers"}
			}
		else
			{ani_stop "wipers"}
		}
	}

	{on "environment_changed"				;// mission script event call to check daytime for vehicle lights
		{call "update_lights"}
	}

	;/*=========== Light FX End  ===========*/

	;/*=========== Damage Begin  ===========*/

	{on "grenade_stick"
		{with_effector
			{link_grenade_to_nearest_human}
		}
	}

	{on "break_glass"
		{volume disable all}
	 	{spawn "glass_break2" volume x}
	 	{delay_ex 0.05 uniq
		 	{bone cut}
	 	}
	}

	{on "explosion"
		{if not able "exploded"
			{tags add "ammo_cooked"}
			{stop_sound "cook_off"}
			{tags remove "ammo_overload"}
			{tags remove "ammo_normal_load"}
			{damage_report "body" "component/empty"}
			{damage_report "engine" "component/empty"}
			{damage_report "trackright" "component/empty"}
			{damage_report "trackleft" "component/empty"}
			{damage_report "turret" "component/empty"}
			{damage_report "gun" "component/empty"}
			{damage_report "mantlet" "component/empty"}
			{able "exploded" 1}
			{call "_explosion"}
			{stop_sound "engine_idle"}
		}
	}

	{on "_explosion" sync
		{tags add "exploded"}
		{call "break_all"}
		{stop_sound "move"}
		{stop_sound "move_end"}
		{stop_sound "engine_full"}
		{stop_sound "engine_stop"}
		{stop_sound "exhaust"}
		{call "engine fullstop"}
		{view stop "fire1"}
		{view stop "fire2"}
		{tags add "destroyed"}
		{kick_entity_manager}
		{volumes disable blast}
		{call "explosion_sound"}
		{if not terrain_fx "bridge"
			{if not terrain_fx "bridge_wood"
				{if not under_water
					{call "spawn_shellhole"}
				}
			}
		}

		{delay_ex 0
			{set "no_burn_volumes" 1}
			{set_skeleton "_xx" sync}
			{impulse up 0.01}
			{ani_play "death"}
			{call "cut_parts"}
		}
		{delay 0.2
			{volumes enable blast}
		}
	}

	{on "break_all"
		{kill_crew}
		{drop_orders}
		{components destroy}
		{reset_manual_control}
		{able select 0}
		{able repaired 0}
		{unlink_trailer}
		{clear_inventory}
	}

	{on "kill_crew"
		{kill_crew}
	}

	{on "explosion_sound"
		{if "crush"
			{start_sound "crush/medium/vehicle"}
		else
			;{start_sound "detonation/explosion_vehicle/"}
			{call "explosion_blastwave"}
		}
	}

	{on "explosion_blastwave"
		{blastwave c4 3 r0 3 r1 8}
	}

	{on "_burn"
		{if not tagged "burned"
			{if not able "exploded"
				{talk "burning"}
				{damage_report "engine" "component/engine_burn"}
				{add_view "smoke_fire_engine" "burn_fire" "FXfire1"}
				{add_view "smoke_engine" "burn_fire_smoke" "FXfire1"}
				;{add_view "smoke_black" "burn_fire" "FXfire1"}
				{view start "burn_fire"}
				{burn time 12}
				{tags add "burned"}
				{play_sound "flame" 1}
				{set "burn_fire" 1}
				{call "stat_break"}
				{tags add "destroyed"}
				{delay 4 2
					{call "check_burn_emit"}
				}
				{delay 10 2
					{components "engine" break}
					{start_sound "crush/medium/metal"}
					{delay 5 4
						{call "hull_destroyed"}
						{call "car_destroyed"}
					}
				}
				{delay 180
					{view start "burn_fire_smoke"}
					{delay 900
						{view stop "burn_fire_smoke"}
					}
					{delay 2
						{view stop "burn_fire"}
					}
					{stop_sound "flame"}
					{set "burn_fire" 0}
					{able "burning" 0}
				}
			}
		}
	}

	{on "burn_emit"
		{if not able "burning"
			{able "burning" 1}
			{set_heater}
			{reset_manual_control}
		}
	}

	{on "burn_volume"
		{if not burned
			{if volume "engine"
				{call "_burn"}
			else volume "engine1"
				{call "_burn"}
			}
		}
	}

	{on "stat_break"
		{if not tagged "body_break"
			{if not "broken"
				{set "broken" 1}
				{if "burn_fire"
					{stat_notify break burned}
				else
					{stat_notify break}				;// stat_notify - used for correct stats gathering
				}
			}
		}
	}

	{on "leave_unit"
		{if work "body"
			{if not tagged "abandoned"
				{if operatable
					{unlink_trailer}
					{reset_manual_control}

					{if not tagged "emit_label"
						{damage_report "crew" "leave_unit"}
						{tags add "emit_label"}
						{crew_emit}
					}
					{delay 2 "emit1"
						{crew_emit}
					}
					{delay 5 "emit2"
						{crew_emit}
					}
					{delay 8 "emit3"
						{crew_emit}
					}
					{delay 11 "emit4"
						{crew_emit}
					}

					{delay 4	{if operatable {tags remove "emit_label"}}}
					{delay 6	{if operatable {tags remove "emit_label"}}}
					{delay 8	{if operatable {tags remove "emit_label"}}}
					{delay 10	{if operatable {tags remove "emit_label"}}}
					{delay 12	{if operatable {tags remove "emit_label"}}}
					{delay 14	{if operatable {tags remove "emit_label"}}}

					{delay 15
						{if not operatable
							{tags add "abandoned"}
							{set "broken" 1}
							{able select 0}
							{able repaired 0}
							{explosive 1}
							{player -1}
							{stat_notify leave}
							{kick_entity_manager}
							{damage_report "gun" "empty"}
							{damage_report "mantlet" "empty"}
							{damage_report "turret" "empty"}
							{damage_report "body" "empty"}
							{damage_report "trackright" "empty"}
							{damage_report "trackleft" "empty"}
							{damage_report "engine" "empty"}

							{delay 3 1
								{explosive 0}
								{tags add "destroyed"}
								{call "explosion"}
							}
						}
					}
				}
			}
		}
	}

	;/*=========== Damage End  ===========*/

	;/*=========== Terrain FX Begin  ===========*/

	(define "terrain_fx"
		{on terrain_fx %0 enter
			{set %0 1}
			{call "check_speed"}
			{view start "on_%1"} 			;// presevers fx for non-tank vehicles
			{view start "on_%1_time_short"}	;// gravel/mud/grass fx
			{play_sound "wheel_%1" 1}
			{call "%3_check"}
			{call "%2_stop_check"}
			{delay 0.1
				{view pause "on_%1_time_short"}
			}
		}
		{on terrain_fx %0 leave
			{set %0 0}
			{view pause "on_%1"}
			{view pause "on_%1_fast"}
			{view pause "on_%1_medium"}
			{view pause "on_%1_slow"}
			{stop_sound "wheel_%1"}
;//			{call "%2_stop_check"}
			{kill_delay "check_speed"}
		}
	)

	("terrain_fx" args "ground"			ground water)
	("terrain_fx" args "sand"			sand water)
	("terrain_fx" args "grass" 			grass water)
	("terrain_fx" args "deep_water" 	water none water)
	("terrain_fx" args "water" 			water none water)
	("terrain_fx" args "shallow_water"	puddle none water)
	("terrain_fx" args "puddle"			puddle water)
	("terrain_fx" args "mud"			mud water)
	("terrain_fx" args "country_road"	ground water)
	("terrain_fx" args "snow"			snow water)
	("terrain_fx" args "ice"			snow none  water)
	("terrain_fx" args "road"			road water)
	("terrain_fx" args "rock"			rock water)
	("terrain_fx" args "bridge"			road water)
	("terrain_fx" args "bridge_wood"	wood water)

	{on "water_check"}
	{on "water_stop_check"}

	{on "check_speed"
		{call "vehicle_velocity"}
		{delay 0.5 "check_speed"
			{call "check_speed"}
		}
	}

	{on "vehicle_velocity"
		(define "terrain_fx2"
			{if %0
				{if velocity 30
					{view start "on_%1_fast"}
					{view pause "on_%1_medium"}
					{view pause "on_%1_slow"}
				else velocity 15
					{view pause "on_%1_fast"}
					{view start "on_%1_medium"}
					{view pause "on_%1_slow"}
				else velocity 0.1
					{view pause "on_%1_fast"}
					{view pause "on_%1_medium"}
					{view start "on_%1_slow"}
				}

			}
		)
		("terrain_fx2" args "ground"		ground)
		("terrain_fx2" args "sand"			sand)
		("terrain_fx2" args "grass" 		grass)
		("terrain_fx2" args "water" 		water)
		("terrain_fx2" args "shallow_water"	puddle)
		("terrain_fx2" args "puddle"		puddle)
		("terrain_fx2" args "mud"			mud)
		("terrain_fx2" args "country_road"	ground)
		("terrain_fx2" args "snow"			snow)
		("terrain_fx2" args "ice"			snow)
		("terrain_fx2" args "road"			road)
		("terrain_fx2" args "rock"			rock)
		("terrain_fx2" args "bridge"		road)
		("terrain_fx2" args "bridge_wood"	wood)
	}

	{on "explosion_delete"
		{set "delete_parts" 1}
		{call "explosion"}
		{delay 20
			{hide 3}
		}
	}

	;/*=========== Terrain FX End  ===========*/

	;/*=========== Weapon FX Begin  ===========*/

	{on "init_gun"
		{add_view	"flash_gun_big"	"flashbarrel3"	"Foresight3"}
		{add_view	"flash_gun_big"	"flashbarrel4"	"Foresight4"}
		{add_view	"flash_gun_big"	"flashbarrel5"	"Foresight5"}
		{add_view	"flash_gun_big"	"flashbarrel6"	"Foresight6"}
		{add_view	"flash_gun_big"	"flashbarrel3"	"Foresight14"}
		{link_sound "bkg"	"weapon/shot/bkg/gun/mg"}
	}

	{on "link_weapon"
		{if effector "stuff weapon"
			{with_effector
				{call "link"}
			}
		}
	}

	(define "on_fire_mgun"
		{on fire "%0"
			{if effector "stuff weapon"
				{with_effector
					{call "fire"}
				}
			else
				{play_sound "bkg"}
				{view start "%1"}
				{delay 0.2
					{view pause "%1"}
				}
			}
		}
	)

	("on_fire_mgun" args mgun flashbarrel3)
	("on_fire_mgun" args mgun1 flashbarrel4)
	("on_fire_mgun" args mgun2 flashbarrel5)
	("on_fire_mgun" args mgun3 flashbarrel6)

	{on "zenite_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "zenite_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "zenite_sand_fx"}
			else terrain_fx "snow"
				{view start "zenite_snow_fx"}
			else terrain_fx "ice"
				{view start "zenite_snow_fx"}
			else
				{view start "zenite_ground_fx"}
			}
		else
			;//do nothing
		}
	}

	{on "minimum_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "minimum_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "minimum_sand_fx"}
			else terrain_fx "snow"
				{view start "minimum_snow_fx"}
			else terrain_fx "ice"
				{view start "minimum_snow_fx"}
			else
				{view start "minimum_ground_fx"}
			}
		else
			;//do nothing
		}
	}

	{on "small_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "small_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "small_sand_fx"}
			else terrain_fx "snow"
				{view start "small_snow_fx"}
			else terrain_fx "ice"
				{view start "small_snow_fx"}
			else
				{view start "small_ground_fx"}
			}
		else
			{if terrain_fx "ground"
				{view start "small_ground_r_fx"}
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "small_sand_r_fx"}
			else terrain_fx "snow"
				{view start "small_snow_fx"}
			else terrain_fx "ice"
				{view start "small_snow_fx"}
			else
				{view start "small_ground_r_fx"}
			}
		}
	}

	{on "medium_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "medium_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "medium_sand_fx"}
			else terrain_fx "snow"
				{view start "medium_snow_fx"}
			else terrain_fx "ice"
				{view start "medium_snow_fx"}
			else
				{view start "medium_ground_fx"}
			}
		else
			{if terrain_fx "ground"
				{view start "medium_ground_r_fx"}
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "medium_sand_r_fx"}
			else terrain_fx "snow"
				{view start "medium_snow_fx"}
			else terrain_fx "ice"
				{view start "medium_snow_fx"}
			else
				{view start "medium_ground_r_fx"}
			}
		}
	}

	{on "large_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "large_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "large_sand_fx"}
			else terrain_fx "snow"
				{view start "large_snow_fx"}
			else terrain_fx "ice"
				{view start "large_snow_fx"}
			else
				{view start "large_ground_fx"}
			}
		else
			{if terrain_fx "ground"
				{view start "large_ground_r_fx"}
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "large_sand_r_fx"}
			else terrain_fx "snow"
				{view start "large_snow_fx"}
			else terrain_fx "ice"
				{view start "large_snow_fx"}
			else
				{view start "large_ground_r_fx"}
			}
		}
	}

	{on "maximum_terrain_fx"
		{if not rain
			{if terrain_fx "ground"
				{view start "maximum_ground_fx"}
			else terrain_fx "mud"
				;//none
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "maximum_sand_fx"}
			else terrain_fx "snow"
				{view start "maximum_snow_fx"}
			else terrain_fx "ice"
				{view start "maximum_snow_fx"}
			else
				{view start "maximum_ground_fx"}
			}
		else
			{if terrain_fx "ground"
				{view start "maximum_ground_r_fx"}
			else terrain_fx "water"
				;//none
			else terrain_fx "shallow_water"
				;//none
			else terrain_fx "sand"
				{view start "maximum_sand_r_fx"}
			else terrain_fx "snow"
				{view start "maximum_snow_fx"}
			else terrain_fx "ice"
				{view start "maximum_snow_fx"}
			else
				{view start "maximum_ground_r_fx"}
			}
		}
	}

	{on "reload"
		{if effector "stuff weapon"
			{set "delete_parts" 1}				;// defined in dummy.inc hides torn magazine after delay.
			{with_effector
				{call "weapon_reload"}			;// defined in stuff.inc
			}
		}
	}

	;/*=========== Weapon FX End  ===========*/

	;/*=========== Constructor Begin  ===========*/

	(define "cannon_sandbag"
		{call "deactivate_vehicle"}
		{start_sound "vehicle/deploy/foxhole"}
		{delay 5
			{start_sound "vehicle/construction/entrench"}
			{spawn %0 %1 x}
			{call "activate_vehicle"}
		}
	)
	{on "construction_sandbag_cannon_shallow"
		("cannon_sandbag" args "sandbag_cannon_shallow" "basis")
	}

	{on "construction_sandbag_cannon_standard"
		("cannon_sandbag" args "sandbag_structured_curved_big" "basis")
	}

	;/*=========== Constructor End  ===========*/

	{on "activate_vehicle"
		{call "update_moveable"}
		{if work "turret" {ik_enable "turret" 1}}
		{if work "gun" {weapon_work "gun" 1}}
		{if work "gun2 "{weapon_work "gun2" 1}}
		{weapon_work "mgun" 1}
		{weapon_work "mgun1" 1}
		{weapon_work "mgun2" 1}
		{weapon_work "mgun3" 1}
		{crew_enable group "gunner" 1}
		{tags remove "deactivated"}
	}

	{on "deactivate_vehicle"
		{if "engine_on"
			{set "toggle_delay" 0}
			{call "engine_toggle"}
		}
		{chassis_work 0}
		{ik_enable "turret" 0}
		{weapon_work "gun" 0}
		{weapon_work "gun2" 0}
		{weapon_work "mgun" 0}
		{weapon_work "mgun1" 0}
		{weapon_work "mgun2" 0}
		{weapon_work "mgun3" 0}
		{weapon_work "mgun4" 0}
		{crew_enable group "gunner" 0}
		{tags add "deactivated"}
	}

	{on "emit_crew"
		{delay 4 "emit1"
			{crew_emit}
		}
		{delay 8 "emit2"
			{crew_emit}
		}
		{delay 12 "emit3"
			{crew_emit}
		}
		{delay 16 "emit4"
			{crew_emit}
		}
	}

	{on animation_end "start"					;// Removes interaction check when animation is over allowing "stop" animation to play
		{set "animated" 0}
	}

	{on animation_end "stop"					;// Removes interaction check when animation is over allowing "start" animation to play
		{set "animated" 0}
	}

	{on animation_event "open"					;// event created by animation
		;{if animation playing forward
			{play_sound "open"}
		;}
	}
	{on animation_event "close"					;// event created by animation
		;{if animation playing backward
			{play_sound "close"}
		;}
	}
	{on animation_event "open_driver"			;// event created by animation
		{play_sound "open_driver"}
	}
	{on animation_event "close_driver"			;// event created by animation
		{play_sound "close_driver"}
	}

	{on "supply_mark"							;// mission helper
		{view start "ammo"}
	}

	(define "rand_cut"
		{if rand 0.5
			{bone cut %0}
		}
	)
	(define "rand_cut_else"
		{if rand 0.5
			{bone cut %0}
		else
			{bone cut %1}
		}
	)
}

;/*=========== Vehicle End ===========*/

;/*=========== General Props Begin ===========*/


{"no_sandbags"
	{on "construction_sandbag_cannon_shallow" overload}
	{on "construction_sandbag_cannon_standard" overload}
}

{"explosion_throw_crew vehicle"
	{on "_explosion"
		{crew_throw_off up 4 1 dir 5 1 turn 0 180 forward 2 rnd_die 0.5}
	}
}

{"emit_on_sink vehicle"
	{on "water_check" overload
		{delay 0.3 "start_check_sinking"
			{call "check_sinking"}
		}
	}
	{on "water_stop_check" overload
		{kill_delay "start_check_sinking"}
		{kill_delay "check_sinking"}
	}
	{on "check_sinking"
		{call "check_water_level"}
		{if able "sinking"
			{call "emit_crew"}
			{reset_manual_control}
			{able "select" 0}
			{call "engine_off"}
		else
			{delay 0.5 "check_sinking"
				{call "check_sinking"}
			}
		}
	}
	{on "check_water_level"
		{if water_level 1.2
			{able "sinking" 1}
		else
			{able "sinking" 0}
		}
	}
}

{"with_gun vehicle"
	{on spawn
		{call "update_gun"}
		{set "rotate_gun" 0}
		{set "rotate_turret" 0}
	}
	{on break
		{if not tagged "exploded"
			{if component "gun"
				{call "update_gun"}
				{tags add "gun_brake"}
				{talk "broken_gun"}
			}
		}
	}
	{on repair
		{if component "gun"
			{call "update_gun"}
			{tags remove "gun_brake"}
		}
	}
	{on "update_gun"
		{if work "gun"
			{weapon_work "gun" 1}
		else
			{weapon_work "gun" 0}
		}
	}
	{on ik begin "gun_rot"
		{kill_delay "rotate_gun"}
		{if not "rotate_gun"
			{play_sound "rotate_gun" 1}
			{set "rotate_gun" 1}
		}
	}
	{on ik end "gun_rot"
		{delay 0.25 "rotate_gun"
			{stop_sound "rotate_gun"}
			{set "rotate_gun" 0}
		}
	}
	{on ik begin "turret"
		{kill_delay "rotate_turret"}
		{if not "rotate_turret"
			{play_sound "rotate_turret" 1}
			{set "rotate_turret" 1}
		}
	}
	{on ik end "turret"
		{delay 0.25 "rotate_turret"
			{stop_sound "rotate_turret"}
			{set "rotate_turret" 0}
		}
	}
	{on crew out		;// stop turret rotation sound when crew leaves gunner seat
		{if not crew_in_place "gunner"
			{if "rotate_turret"
				{stop_sound "rotate_turret"}
			}
		}
	}
}

{"with_gun vehicle open_top"	;// Adds animation calls when aiming gun on open topped vehicles
	{on "pre_cookoff" 		;//prevents open top vehicles from cooking off while they dont have animation
		{call "explosion"}
	}
	{on ik begin "gun_rot" overload
		{kill_delay "rotate_gun"}
		{if not "rotate_gun"
			{play_sound "rotate_gun" 1}
			{set "rotate_gun" 1}
			{ani_play "aim_v" loop resume}
		}
	}
	{on ik end "gun_rot" overload
		{delay 0.25 "rotate_gun"
			{stop_sound "rotate_gun"}
			{set "rotate_gun" 0}
			{ani_stop "aim_v"}
		}
	}
	{on ik begin "turret" overload
		{kill_delay "rotate_turret"}
		{if not "rotate_turret"
			{play_sound "rotate_turret" 1}
			{set "rotate_turret" 1}
			{ani_play "aim_h" loop resume}
		}
	}
	{on ik end "turret" overload
		{delay 0.25 "rotate_turret"
			{stop_sound "rotate_turret"}
			{set "rotate_turret" 0}
			{ani_stop "aim_h"}
		}
	}
}

{"vehicle multi_board"	;// used for vehicles like transport trucks where multiple crew/passengers use 1 door to board
	{on spawn
		{if tagged "door_open"
			{ani_wind "open_back" end}
		}
	}
	{on animation_event "board"
		{if tagged "door_open"
			{kill_delay "boarding"}
			{delay 5 "boarding"
				{if operatable
					{tags remove "door_open"}
					{ani_play "close_back" 1}
				}
			}
		else
			{ani_play "open_back" 1}
			{tags add "door_open"}
			{delay 5 "boarding"
				{if operatable
					{tags remove "door_open"}
					{ani_play "close_back" 1}
				}
			}
		}
	}
}

{"open_with_land"
	{on "open"
		{ani_stop "drift"}
		{ani_play "dooropen" callback}
		{land on}
		{chassis_work 0}
		{physics_work 0}
		{area enable}
	}
	{on "close"
		{ani_play "dooropen" -1 callback}
		{ani_play "drift" loop}
		{land off}
		{chassis_work 1}
		{physics_work 1}
		{area disable}
	}
	{on animation_end "dooropen"
		{update_visibility_patch}
	}
}

{"open_without_land"
	{on "open"
		{ani_stop "drift"}
		{ani_play "dooropen" callback}
	}
	{on "close"
		{ani_play "dooropen" -1 callback}
		{ani_play "drift" loop}
	}
	{on animation_end "dooropen"
		{update_visibility_patch}
	}
}

{"amphibia vehicle"
	{on spawn
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center1"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center2"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center3"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center4"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center5"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center6"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center7"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center8"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center9"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_small_center10"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_norm_front1"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_norm_front2"}
		{add_view	"wash_ship_stop"	"on_stead"	"FX_bulb_norm_engine1"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center1"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center2"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center3"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center4"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center5"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center6"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center7"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center8"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center9"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_center10"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_back1"}
		{add_view	"wash_ship_motion"	"on_move"	"FX_bulb_small_back2"}
		{add_view	"wash_front_small"	"on_move"	"FX_bulb_small_front1"}
		{add_view	"wash_front_small"	"on_move"	"FX_bulb_small_front2"}
		{add_view	"wash_front_norm"	"on_move"	"FX_bulb_norm_front1"}
		{add_view	"wash_front_norm"	"on_move"	"FX_bulb_norm_front2"}
		{add_view	"wash_ship_motion_norm"	"on_move"	"FX_bulb_norm_front1"}
		{add_view	"wash_ship_motion_norm"	"on_move"	"FX_bulb_norm_front2"}
		{add_view	"wash_engine_small"	"on_move"	"FX_bulb_small_engine1"}
		{add_view	"wash_engine_norm"	"on_move"	"FX_bulb_norm_engine1"}
	}
	{on terrain_pp "deep_water" enter
		{set "in_water" 1}
		{call "update_bulbs"}
	}
	{on terrain_pp "water" enter
		{set "in_water" 1}
		{call "update_bulbs"}
	}
	{on terrain_pp "shallow_water" enter
		{set "in_water" 1}
		{call "update_bulbs"}
	}
	{on terrain_pp "shallow_water" leave
		{set "in_water" 0}
		{call "update_bulbs"}
	}
	{on "update_bulbs"
		{if "in_water"
			{if not velocity 0.001
				{view start "on_stead"}
				{view pause "on_move"}
				{ani_stop "move"}
			else
				{view start "on_move"}
				{ani_play "move" loop}
				{view pause "on_stead"}
			}
		else
			{view pause "on_stead"}
			{view pause "on_move"}
			{ani_stop "move"}
		}
	}
	{on "check_water_level" overload}
	{on move on
		{call "update_bulbs"}
	}
	{on move off
		{call "update_bulbs"}
	}
}

{"flame vehicle"
	{on spawn
		{link_sound "start"		"weapon/shot/flame/vehicle/vehicle_flamer_start"}
		{link_sound "loop"		"weapon/shot/flame/vehicle/vehicle_flamer_loop"}
		{link_sound "end"		"weapon/shot/flame/vehicle/vehicle_flamer_stop"}
	}
	(define "flame_weapon"

		{on fire "%0" overload
			{spawn "fx_piece_flame_%1" "foresight%2" x
				{impulse forward %3 rnd_dir 0.01} ;40
				{gravity 0.65}
				{linear_damping 0}
			}
			{view start	"flame"}
			{delay 0.1
				{view pause	"flame"}
			}
			{if tagged "firing"
				{call "play_shoot_loop"}
			else
				{play_sound "start"}
				{tags add "looping"}
			}

			{tags add "firing"}
			{delay 0.125
				{tags remove "firing"}
				{tags remove "looping"}
				{play_sound "end"}
				{delay 0.05
					{stop_sound "loop"}
				}
			}
		}

		{on "play_shoot_loop"
			{if tagged "looping"
				{play_sound "loop" 1}
				{tags remove "looping"}
			}
		}
	)
}

;/*=========== General Props End ===========*/

{"diesel"	;// alternate fuel type
	{on spawn
		{tags add "add_exhaustview"}
	}
	{on "add_exhaustview" overload
		{add_view	"exhaust_dark_big_diesel"		"exhaust"			   "FXsmoke1"} ;// engine exhaust
		{add_view	"exhaust_dark_big_diesel"		"exhaust"			   "FXsmoke2"} ;// engine exhaust
		{add_view	"exhaust_dark_small2_diesel"	"exhaust2"			   "FXsmoke1"} ;// idle exhaust
		{add_view	"exhaust_dark_small2_diesel"	"exhaust2"			   "FXsmoke2"} ;// idle exhaust
	}
}
{"td"} 						;// used for minimap icons
{"spg"} 					;// used for minimap icons
{"sandbag_tank_near"} 		;// used for constructor
{"sandbag_tank_far"} 		;// used for constructor
{"cannon_sandbag_small"} 	;// used for constructor
{"cannon_sandbag_standard"} ;// used for constructor
{"no_sandbags"}		 		;// used for constructor
