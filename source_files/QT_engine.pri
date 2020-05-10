# This file is automatically generated by synchronize.py
# To change the list of source files run synchronize.py

HEADERS += engine/action/absorb.hpp
HEADERS += engine/action/action_callback.hpp
HEADERS += engine/action/attack.hpp
HEADERS += engine/action/dbc_proc_callback.hpp
HEADERS += engine/action/dot.hpp
HEADERS += engine/action/heal.hpp
HEADERS += engine/action/residual_action.hpp
HEADERS += engine/action/sc_action.hpp
HEADERS += engine/action/sc_action_state.hpp
HEADERS += engine/action/sequence.hpp
HEADERS += engine/action/snapshot_stats.hpp
HEADERS += engine/action/spell.hpp
HEADERS += engine/action/spell_base.hpp
HEADERS += engine/action/variable.hpp
HEADERS += engine/buff/sc_buff.hpp
HEADERS += engine/class_modules/class_module.hpp
HEADERS += engine/class_modules/paladin/sc_paladin.hpp
HEADERS += engine/class_modules/priest/sc_priest.hpp
HEADERS += engine/class_modules/warlock/sc_warlock.hpp
HEADERS += engine/class_modules/warlock/sc_warlock_pets.hpp
HEADERS += engine/config.hpp
HEADERS += engine/dbc/azerite.hpp
HEADERS += engine/dbc/client_data.hpp
HEADERS += engine/dbc/data_definitions.hh
HEADERS += engine/dbc/data_enums.hh
HEADERS += engine/dbc/dbc.hpp
HEADERS += engine/dbc/gem_data.hpp
HEADERS += engine/dbc/item_armor.hpp
HEADERS += engine/dbc/item_bonus.hpp
HEADERS += engine/dbc/item_database.hpp
HEADERS += engine/dbc/item_effect.hpp
HEADERS += engine/dbc/item_naming.hpp
HEADERS += engine/dbc/item_scaling.hpp
HEADERS += engine/dbc/item_weapon.hpp
HEADERS += engine/dbc/rand_prop_points.hpp
HEADERS += engine/dbc/sc_spell_info.hpp
HEADERS += engine/dbc/specialization.hpp
HEADERS += engine/dbc/spell_item_enchantment.hpp
HEADERS += engine/dbc/spell_query/spell_data_expr.hpp
HEADERS += engine/interfaces/bcp_api.hpp
HEADERS += engine/interfaces/sc_http.hpp
HEADERS += engine/interfaces/sc_http_curl.hpp
HEADERS += engine/interfaces/sc_http_wininet.hpp
HEADERS += engine/interfaces/sc_js.hpp
HEADERS += engine/interfaces/wowhead.hpp
HEADERS += engine/item/enchants.hpp
HEADERS += engine/item/item.hpp
HEADERS += engine/item/item_targetdata_initializer.hpp
HEADERS += engine/item/special_effect.hpp
HEADERS += engine/lib/gsl-lite/gsl-lite.hpp
HEADERS += engine/lib/rapidxml/rapidxml.hpp
HEADERS += engine/lib/rapidxml/rapidxml_print.hpp
HEADERS += engine/lib/tcb/span.hpp
HEADERS += engine/player/action_priority_list.hpp
HEADERS += engine/player/action_variable.hpp
HEADERS += engine/player/actor.hpp
HEADERS += engine/player/actor_target_data.hpp
HEADERS += engine/player/artifact_data.hpp
HEADERS += engine/player/assessor.hpp
HEADERS += engine/player/azerite_data.hpp
HEADERS += engine/player/consumable.hpp
HEADERS += engine/player/darkmoon_deck.hpp
HEADERS += engine/player/effect_callbacks.hpp
HEADERS += engine/player/expansion_effects.hpp
HEADERS += engine/player/gear_stats.hpp
HEADERS += engine/player/ground_aoe.hpp
HEADERS += engine/player/instant_absorb.hpp
HEADERS += engine/player/pet.hpp
HEADERS += engine/player/pet_spawner.hpp
HEADERS += engine/player/pet_spawner_impl.hpp
HEADERS += engine/player/player_collected_data.hpp
HEADERS += engine/player/player_demise_event.hpp
HEADERS += engine/player/player_event.hpp
HEADERS += engine/player/player_processed_report_information.hpp
HEADERS += engine/player/player_resources.hpp
HEADERS += engine/player/player_scaling.hpp
HEADERS += engine/player/player_stat_cache.hpp
HEADERS += engine/player/player_talent_points.hpp
HEADERS += engine/player/rating.hpp
HEADERS += engine/player/sample_data_helper.hpp
HEADERS += engine/player/sc_actor_pair.hpp
HEADERS += engine/player/sc_player.hpp
HEADERS += engine/player/scaling_metric_data.hpp
HEADERS += engine/player/set_bonus.hpp
HEADERS += engine/player/spawner_base.hpp
HEADERS += engine/player/stats.hpp
HEADERS += engine/player/target_specific.hpp
HEADERS += engine/player/unique_gear.hpp
HEADERS += engine/player/weapon.hpp
HEADERS += engine/report/sc_highchart.hpp
HEADERS += engine/report/sc_report.hpp
HEADERS += engine/sc_enums.hpp
HEADERS += engine/sc_timespan.hpp
HEADERS += engine/sim/artifact_power.hpp
HEADERS += engine/sim/benefit.hpp
HEADERS += engine/sim/event.hpp
HEADERS += engine/sim/event_manager.hpp
HEADERS += engine/sim/gain.hpp
HEADERS += engine/sim/iteration_data_entry.hpp
HEADERS += engine/sim/plot.hpp
HEADERS += engine/sim/proc.hpp
HEADERS += engine/sim/progress_bar.hpp
HEADERS += engine/sim/raid_event.hpp
HEADERS += engine/sim/real_ppm.hpp
HEADERS += engine/sim/reforge_plot.hpp
HEADERS += engine/sim/sc_cooldown.hpp
HEADERS += engine/sim/sc_expressions.hpp
HEADERS += engine/sim/sc_option.hpp
HEADERS += engine/sim/sc_profileset.hpp
HEADERS += engine/sim/sc_sim.hpp
HEADERS += engine/sim/scale_factor_control.hpp
HEADERS += engine/sim/shuffled_rng.hpp
HEADERS += engine/sim/sim_control.hpp
HEADERS += engine/sim/sim_ostream.hpp
HEADERS += engine/sim/uptime.hpp
HEADERS += engine/sim/x6_pantheon.hpp
HEADERS += engine/simulationcraft.hpp
HEADERS += engine/util/cache.hpp
HEADERS += engine/util/concurrency.hpp
HEADERS += engine/util/generic.hpp
HEADERS += engine/util/git_info.hpp
HEADERS += engine/util/io.hpp
HEADERS += engine/util/plot_data.hpp
HEADERS += engine/util/rng.hpp
HEADERS += engine/util/sample_data.hpp
HEADERS += engine/util/sc_resourcepaths.hpp
HEADERS += engine/util/scoped_callback.hpp
HEADERS += engine/util/span.hpp
HEADERS += engine/util/stopwatch.hpp
HEADERS += engine/util/timeline.hpp
HEADERS += engine/util/util.hpp
HEADERS += engine/util/vector_with_callback.hpp
HEADERS += engine/util/xml.hpp

SOURCES += engine/action/absorb.cpp
SOURCES += engine/action/action_callback.cpp
SOURCES += engine/action/dbc_proc_callback.cpp
SOURCES += engine/action/heal.cpp
SOURCES += engine/action/residual_action.cpp
SOURCES += engine/action/sc_action.cpp
SOURCES += engine/action/sc_action_state.cpp
SOURCES += engine/action/sc_attack.cpp
SOURCES += engine/action/sc_dot.cpp
SOURCES += engine/action/sc_spell.cpp
SOURCES += engine/action/sc_stats.cpp
SOURCES += engine/action/sequence.cpp
SOURCES += engine/action/snapshot_stats.cpp
SOURCES += engine/action/variable.cpp
SOURCES += engine/buff/sc_buff.cpp
SOURCES += engine/class_modules/paladin/sc_paladin.cpp
SOURCES += engine/class_modules/paladin/sc_paladin_holy.cpp
SOURCES += engine/class_modules/paladin/sc_paladin_protection.cpp
SOURCES += engine/class_modules/paladin/sc_paladin_retribution.cpp
SOURCES += engine/class_modules/priest/sc_priest.cpp
SOURCES += engine/class_modules/priest/sc_priest_discipline.cpp
SOURCES += engine/class_modules/priest/sc_priest_holy.cpp
SOURCES += engine/class_modules/priest/sc_priest_shadow.cpp
SOURCES += engine/class_modules/sc_death_knight.cpp
SOURCES += engine/class_modules/sc_demon_hunter.cpp
SOURCES += engine/class_modules/sc_druid.cpp
SOURCES += engine/class_modules/sc_enemy.cpp
SOURCES += engine/class_modules/sc_hunter.cpp
SOURCES += engine/class_modules/sc_mage.cpp
SOURCES += engine/class_modules/sc_monk.cpp
SOURCES += engine/class_modules/sc_rogue.cpp
SOURCES += engine/class_modules/sc_shaman.cpp
SOURCES += engine/class_modules/sc_warrior.cpp
SOURCES += engine/class_modules/warlock/sc_warlock.cpp
SOURCES += engine/class_modules/warlock/sc_warlock_affliction.cpp
SOURCES += engine/class_modules/warlock/sc_warlock_demonology.cpp
SOURCES += engine/class_modules/warlock/sc_warlock_destruction.cpp
SOURCES += engine/class_modules/warlock/sc_warlock_pets.cpp
SOURCES += engine/dbc/azerite.cpp
SOURCES += engine/dbc/gem_data.cpp
SOURCES += engine/dbc/item_armor.cpp
SOURCES += engine/dbc/item_bonus.cpp
SOURCES += engine/dbc/item_effect.cpp
SOURCES += engine/dbc/item_naming.cpp
SOURCES += engine/dbc/item_scaling.cpp
SOURCES += engine/dbc/item_weapon.cpp
SOURCES += engine/dbc/rand_prop_points.cpp
SOURCES += engine/dbc/sc_const_data.cpp
SOURCES += engine/dbc/sc_data.cpp
SOURCES += engine/dbc/sc_item_data.cpp
SOURCES += engine/dbc/sc_item_data_import_noptr.cpp
SOURCES += engine/dbc/sc_item_data_import_ptr.cpp
SOURCES += engine/dbc/sc_spell_data.cpp
SOURCES += engine/dbc/sc_spell_info.cpp
SOURCES += engine/dbc/spell_item_enchantment.cpp
SOURCES += engine/interfaces/bcp_api.cpp
SOURCES += engine/interfaces/sc_http.cpp
SOURCES += engine/interfaces/sc_http_curl.cpp
SOURCES += engine/interfaces/sc_http_wininet.cpp
SOURCES += engine/interfaces/sc_js.cpp
SOURCES += engine/interfaces/wowhead.cpp
SOURCES += engine/item/enchants.cpp
SOURCES += engine/item/item.cpp
SOURCES += engine/item/item_targetdata_initializer.cpp
SOURCES += engine/item/special_effect.cpp
SOURCES += engine/lib/fmt/format.cpp
SOURCES += engine/lib/fmt/os.cpp
SOURCES += engine/player/action_priority_list.cpp
SOURCES += engine/player/action_variable.cpp
SOURCES += engine/player/actor.cpp
SOURCES += engine/player/actor_target_data.cpp
SOURCES += engine/player/artifact_data.cpp
SOURCES += engine/player/azerite_data.cpp
SOURCES += engine/player/darkmoon_deck.cpp
SOURCES += engine/player/ground_aoe.cpp
SOURCES += engine/player/instant_absorb.cpp
SOURCES += engine/player/player_event.cpp
SOURCES += engine/player/player_stat_cache.cpp
SOURCES += engine/player/player_talent_points.cpp
SOURCES += engine/player/sc_consumable.cpp
SOURCES += engine/player/sc_pet.cpp
SOURCES += engine/player/sc_player.cpp
SOURCES += engine/player/sc_proc.cpp
SOURCES += engine/player/sc_set_bonus.cpp
SOURCES += engine/player/sc_unique_gear.cpp
SOURCES += engine/player/spawner_base.cpp
SOURCES += engine/player/target_specific.cpp
SOURCES += engine/player/unique_gear_bfa.cpp
SOURCES += engine/player/unique_gear_legion.cpp
SOURCES += engine/report/sc_chart.cpp
SOURCES += engine/report/sc_color.cpp
SOURCES += engine/report/sc_gear_weights.cpp
SOURCES += engine/report/sc_highchart.cpp
SOURCES += engine/report/sc_report.cpp
SOURCES += engine/report/sc_report_html_player.cpp
SOURCES += engine/report/sc_report_html_sim.cpp
SOURCES += engine/report/sc_report_json.cpp
SOURCES += engine/report/sc_report_text.cpp
SOURCES += engine/sim/artifact_power.cpp
SOURCES += engine/sim/event_manager.cpp
SOURCES += engine/sim/proc.cpp
SOURCES += engine/sim/real_ppm.cpp
SOURCES += engine/sim/sc_cooldown.cpp
SOURCES += engine/sim/sc_event.cpp
SOURCES += engine/sim/sc_expressions.cpp
SOURCES += engine/sim/sc_gear_stats.cpp
SOURCES += engine/sim/sc_option.cpp
SOURCES += engine/sim/sc_plot.cpp
SOURCES += engine/sim/sc_profileset.cpp
SOURCES += engine/sim/sc_progress_bar.cpp
SOURCES += engine/sim/sc_raid_event.cpp
SOURCES += engine/sim/sc_reforge_plot.cpp
SOURCES += engine/sim/sc_sim.cpp
SOURCES += engine/sim/scale_factor_control.cpp
SOURCES += engine/sim/shuffled_rng.cpp
SOURCES += engine/sim/uptime_benefit.cpp
SOURCES += engine/sim/x6_pantheon.cpp
SOURCES += engine/util/concurrency.cpp
SOURCES += engine/util/git_info.cpp
SOURCES += engine/util/io.cpp
SOURCES += engine/util/rng.cpp
SOURCES += engine/util/sample_data.cpp
SOURCES += engine/util/stopwatch.cpp
SOURCES += engine/util/timeline.cpp
SOURCES += engine/util/util.cpp
SOURCES += engine/util/xml.cpp