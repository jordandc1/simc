#include "simulationcraft.hpp"
#include "sc_warlock.hpp"

namespace warlock {
  namespace pets {
    namespace felguard {
      struct legion_strike_t : public warlock_pet_melee_attack_t {
        legion_strike_t(warlock_pet_t* p, const std::string& options_str) : warlock_pet_melee_attack_t(p, "Legion Strike") {
          parse_options(options_str);
          aoe = -1;
          weapon = &(p->main_hand_weapon);
          base_dd_min = base_dd_max = p->composite_melee_attack_power() * data().effectN(1).ap_coeff();
        }
      };

      struct axe_toss_t : public warlock_pet_spell_t {
        axe_toss_t(warlock_pet_t* p, const std::string& options_str) : warlock_pet_spell_t("Axe Toss", p, p -> find_spell(89766)) {
          parse_options(options_str);
        }

        void execute() override {
          warlock_pet_spell_t::execute();
          p()->trigger_sephuzs_secret(execute_state, MECHANIC_STUN);
        }
      };

      struct felstorm_tick_t : public warlock_pet_melee_attack_t {
        felstorm_tick_t(warlock_pet_t* p, const spell_data_t& s) : warlock_pet_melee_attack_t("felstorm_tick", p, s.effectN(1).trigger()) {
          aoe = -1;
          background = true;
          weapon = &(p->main_hand_weapon);
          base_dd_min = base_dd_max = p->composite_melee_attack_power() * data().effectN(1).ap_coeff();
        }

        double action_multiplier() const override
        {
          double m = warlock_pet_melee_attack_t::action_multiplier();

          if (p()->buffs.demonic_strength->check())
          {
            m *= p()->buffs.demonic_strength->default_value;
          }

          return m;
        }
      };

      struct felstorm_t : public warlock_pet_melee_attack_t {
        felstorm_t(warlock_pet_t* p, const std::string& options_str) : warlock_pet_melee_attack_t("felstorm", p, p -> find_spell(89751)) {
          parse_options(options_str);
          tick_zero = true;
          hasted_ticks = true;
          may_miss = false;
          may_crit = false;
          channeled = true;

          dynamic_tick_action = true;
          tick_action = new felstorm_tick_t(p, data());
        }

        timespan_t composite_dot_duration(const action_state_t* s) const override
        {
          return s->action->tick_time(s) * 5.0;
        }
      };

      struct demonic_strength_t : public warlock_pet_melee_attack_t {
        bool queued;

        demonic_strength_t(warlock_pet_t* p, const std::string& options_str) :
          warlock_pet_melee_attack_t("demonic_strength_felstorm", p, p -> find_spell(89751)),
          queued(false)
        {
          parse_options(options_str);
          tick_zero = true;
          hasted_ticks = true;
          may_miss = false;
          may_crit = false;
          channeled = true;

          dynamic_tick_action = true;
          tick_action = new felstorm_tick_t(p, data());
        }

        timespan_t composite_dot_duration(const action_state_t* s) const override
        {
          return s->action->tick_time(s) * 5.0;
        }

        double action_multiplier() const override
        {
          double m = warlock_pet_melee_attack_t::action_multiplier();

          if (p()->buffs.demonic_strength->check())
          {
            m *= p()->buffs.demonic_strength->default_value;
          }

          return m;
        }

        void cancel() override {
          warlock_pet_melee_attack_t::cancel();
          get_dot()->cancel();
        }

        void execute() override
        {
          warlock_pet_melee_attack_t::execute();
          queued = false;
          p()->melee_attack->cancel();
        }

        void last_tick(dot_t* d) override {
          warlock_pet_melee_attack_t::last_tick(d);

          p()->buffs.demonic_strength->expire();
        }

        bool ready() override {
          if (!queued)
            return false;
          return warlock_pet_melee_attack_t::ready();
        }
      };

      struct soul_strike_t : public warlock_pet_melee_attack_t {
        soul_strike_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("Soul Strike", p, p->find_spell(267964)) {
          background = true;
        }

        bool ready() override {
          if (p()->pet_type != PET_FELGUARD) return false;
          return warlock_pet_melee_attack_t::ready();
        }
      };

      felguard_pet_t::felguard_pet_t(sim_t* sim, warlock_t* owner, const std::string& name) :
        warlock_pet_t(sim, owner, name, PET_FELGUARD, name != "felguard") {
        action_list_str = "travel";
        action_list_str += "/demonic_strength_felstorm";
        action_list_str += "/felstorm";
        action_list_str += "/legion_strike,if=energy>=100";
      }

      bool felguard_pet_t::create_actions() {
        auto r = warlock_pet_t::create_actions();

        active.demonic_strength_felstorm  = find_action("demonic_strength_felstorm");
        assert( active.demonic_strength_felstorm );

        return r;
      }

      void felguard_pet_t::init_base_stats() {
        warlock_pet_t::init_base_stats();

        melee_attack = new warlock_pet_melee_t(this);
        special_action = new axe_toss_t(this,"");
      }

      double felguard_pet_t::composite_player_multiplier(school_e school) const {
        double m = warlock_pet_t::composite_player_multiplier(school);
        m *= 1.1;
        return m;
      }

      action_t* felguard_pet_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "legion_strike") return new legion_strike_t(this, options_str);
        if (name == "demonic_strength_felstorm") return new demonic_strength_t(this, options_str);
        if (name == "felstorm") return new felstorm_t(this, options_str);
        if (name == "axe_toss") return new axe_toss_t(this, options_str);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace wild_imp {
      struct fel_firebolt_t : public warlock_pet_spell_t
      {
        fel_firebolt_t(warlock_pet_t* p) : warlock_pet_spell_t("fel_firebolt", p, p -> find_spell(104318))
        {
        }

        bool ready() override
        {
          if (!p()->resource_available(p()->primary_resource(), 20) & !p()->o()->buffs.demonic_power->check())
            p()->demise();

          return spell_t::ready();
        }

        double cost() const override
        {
          double c = warlock_pet_spell_t::cost();

          if (p()->o()->buffs.demonic_power->check())
          {
            c *= 1.0 + p()->o()->buffs.demonic_power->data().effectN(4).percent();
          }

          return c;
        }
      };

      wild_imp_pet_t::wild_imp_pet_t(sim_t* sim, warlock_t* owner) : warlock_pet_t(sim, owner, "wild_imp", PET_WILD_IMP),
          firebolt(),
          isnotdoge()
      {
      }

      void wild_imp_pet_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();

        action_list_str = "fel_firebolt";

        resources.base[RESOURCE_ENERGY] = 100;
        resources.base_regen_per_second[RESOURCE_ENERGY] = 0;
      }

      action_t* wild_imp_pet_t::create_action(const std::string& name,const std::string& options_str)
      {
        if (name == "fel_firebolt")
        {
          assert( firebolt == nullptr ); // TODO: Check if we really want a non-background action stored in a pet-level
          // action?
          firebolt = new fel_firebolt_t(this);
          return firebolt;
        }

        return warlock_pet_t::create_action(name, options_str);
      }

      void wild_imp_pet_t::arise()
      {
        warlock_pet_t::arise();

        o()->buffs.wild_imps->increment();
        if (isnotdoge)
        {
          firebolt->cooldown->start(timespan_t::from_millis(rng().range(0, 500)));
        }
      }

      void wild_imp_pet_t::demise() {
        warlock_pet_t::demise();

        o()->buffs.wild_imps->decrement();
        o()->buffs.demonic_core->trigger(1, buff_t::DEFAULT_VALUE(), o()->spec.demonic_core->effectN(1).percent());
      }
    }

    namespace dreadstalker {
      struct dreadbite_t : public warlock_pet_melee_attack_t
      {
        double t21_4pc_increase;

        dreadbite_t(warlock_pet_t* p) :
          warlock_pet_melee_attack_t("Dreadbite", p, p -> find_spell(205196))
        {
          weapon = &(p->main_hand_weapon);
          if (p->o()->talents.dreadlash->ok())
          {
            aoe = -1;
            radius = 8;
          }
          t21_4pc_increase = p->o()->sets->set(WARLOCK_DEMONOLOGY, T21, B4)->effectN(1).percent();
        }

        bool ready() override
        {
          if (p()->dreadbite_executes <= 0)
            return false;

          return warlock_pet_melee_attack_t::ready();
        }

        double action_multiplier() const override
        {
          double m = warlock_pet_melee_attack_t::action_multiplier();

          m *= 1.2; //until I can figure out wtf is going on with this spell

          if (p()->o()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T21, B4) && p()->bites_executed == 1)
            m *= 1.0 + t21_4pc_increase;

          if (p()->o()->talents.dreadlash->ok())
          {
            m *= 1.0 + p()->o()->talents.dreadlash->effectN(1).percent();
          }

          return m;
        }

        void execute() override
        {
          warlock_pet_melee_attack_t::execute();

          p()->dreadbite_executes--;
        }

        void impact(action_state_t* s) override
        {
          warlock_pet_melee_attack_t::impact(s);

          p()->bites_executed++;
        }
      };

      dreadstalker_t::dreadstalker_t(sim_t* sim, warlock_t* owner) : warlock_pet_t(sim, owner, "dreadstalker", PET_DREADSTALKER)
      {
        action_list_str = "travel/dreadbite";
        regen_type = REGEN_DISABLED;
        owner_coeff.ap_from_sp = 0.33;
      }

      void dreadstalker_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        resources.base[RESOURCE_ENERGY] = 0;
        resources.base_regen_per_second[RESOURCE_ENERGY] = 0;
        melee_attack = new warlock_pet_melee_t(this);
      }

      void dreadstalker_t::arise()
      {
        warlock_pet_t::arise();

        o()->buffs.dreadstalkers->set_duration(o()->find_spell(193332)->duration());
        o()->buffs.dreadstalkers->trigger();

        dreadbite_executes = 1;
        bites_executed = 0;

        if (o()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T21, B4))
          t21_4pc_reset = false;
      }

      void dreadstalker_t::demise() {
        warlock_pet_t::demise();

        o()->buffs.dreadstalkers->decrement();
        o()->buffs.demonic_core->trigger(1, buff_t::DEFAULT_VALUE(), o()->spec.demonic_core->effectN(2).percent());
      }

      action_t* dreadstalker_t::create_action(const std::string& name, const std::string& options_str)
      {
        if (name == "dreadbite") return new dreadbite_t(this);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace vilefiend
    {
      struct bile_spit_t : public warlock_pet_spell_t
      {
        bile_spit_t(warlock_pet_t* p) : warlock_pet_spell_t("bile_spit", p, p -> find_spell(267997))
        {
          tick_may_crit = true;
        }
      };
      struct headbutt_t : public warlock_pet_melee_attack_t {
        headbutt_t(warlock_pet_t* p) : warlock_pet_melee_attack_t(p, "Headbutt") {
          cooldown->duration = timespan_t::from_seconds(5);
        }
      };

      vilefiend_t::vilefiend_t(sim_t* sim, warlock_t* owner) : warlock_pet_t(sim, owner, "vilefiend", PET_VILEFIEND)
      {
        action_list_str += "travel/headbutt";
        owner_coeff.ap_from_sp = 0.46;
      }

      void vilefiend_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        melee_attack = new warlock_pet_melee_t(this);
      }
      
      action_t* vilefiend_t::create_action(const std::string& name, const std::string& options_str)
      {
        if ( name == "bile_spit" ) return new bile_spit_t(this);
        if ( name == "headbutt" ) return new headbutt_t(this);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace demonic_tyrant {
      struct demonfire_blast_t : public warlock_pet_spell_t
      {
        demonfire_blast_t(warlock_pet_t* p, const std::string& options_str) : warlock_pet_spell_t("demonfire_blast", p, p -> find_spell(265279))
        {
          parse_options(options_str);
        }

        double action_multiplier() const override
        {
          double m = warlock_pet_spell_t::action_multiplier();

          if (p()->buffs.demonic_consumption->check())
          {
            m *= 1.0 + p()->buffs.demonic_consumption->check_stack_value();
          }

          return m;
        }
      };

      struct demonfire_t : public warlock_pet_spell_t
      {
        demonfire_t(warlock_pet_t* p, const std::string& options_str) : warlock_pet_spell_t("demonfire", p, p -> find_spell(270481))
        {
          parse_options(options_str);
        }

        double action_multiplier() const override
        {
          double m = warlock_pet_spell_t::action_multiplier();

          if (p()->buffs.demonic_consumption->check())
          {
            m *= 1.0 + p()->buffs.demonic_consumption->check_stack_value();
          }

          return m;
        }
      };

      demonic_tyrant_t::demonic_tyrant_t(sim_t* sim, warlock_t* owner, const std::string& name) :
        warlock_pet_t(sim, owner, name, PET_DEMONIC_TYRANT, name != "demonic_tyrant") {
        action_list_str += "/sequence,name=rotation:demonfire_blast:demonfire:demonfire:demonfire";
        action_list_str += "/restart_sequence,name=rotation";
      }

      void demonic_tyrant_t::init_base_stats() {
        warlock_pet_t::init_base_stats();
      }

      action_t* demonic_tyrant_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "demonfire") return new demonfire_t(this,options_str);
        if (name == "demonfire_blast") return new demonfire_blast_t(this,options_str);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace shivarra {
      struct multi_slash_damage_t : public warlock_pet_melee_attack_t
      {
        multi_slash_damage_t(warlock_pet_t* p, int slash_num) : warlock_pet_melee_attack_t("multi-slash-" + std::to_string(slash_num), p, p -> find_spell(272172))
        {
          attack_power_mod.direct = data().effectN(slash_num).ap_coeff();
        }
      };

      struct multi_slash_t : public warlock_pet_melee_attack_t
      {
        std::array<multi_slash_damage_t*,4> slashs;

        multi_slash_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("multi-slash", p, p -> find_spell(272172))
        {
          for (unsigned i = 0; i < slashs.size(); ++i)
          {
            slashs[i] = new multi_slash_damage_t(p, i);
            add_child(slashs[i]);
          }
        }

        void execute() override
        {
          cooldown->start(timespan_t::from_millis(rng().range(7000, 9000)));

          for(auto& slash : slashs)
          {
            slash->execute();
          }
        }
      };

      shivarra_t::shivarra_t(sim_t* sim, warlock_t* owner, const std::string& name) :
          warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "shivarra"),
          multi_slash()
      {
        action_list_str = "travel/multi_slash";
        owner_coeff.ap_from_sp = 0.065;
      }

      void shivarra_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        off_hand_weapon = main_hand_weapon;
        melee_attack = new warlock_pet_melee_t(this, 2.0);
      }

      void shivarra_t::arise()
      {
        warlock_pet_t::arise();
        multi_slash->cooldown->start(timespan_t::from_millis(rng().range(3500, 5100)));
      }

      action_t* shivarra_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "multi_slash")
        {
          assert(multi_slash == nullptr);
          multi_slash = new multi_slash_t(this);
          return multi_slash;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace darkhound {
      struct fel_bite_t : public warlock_pet_melee_attack_t {
        fel_bite_t(warlock_pet_t* p) : warlock_pet_melee_attack_t(p, "Fel Bite") {
        }

        void execute() override {
          warlock_pet_melee_attack_t::execute();
          cooldown->start(timespan_t::from_millis(rng().range(4500, 6500)));
        }
      };

      darkhound_t::darkhound_t(sim_t* sim, warlock_t* owner, const std::string& name) :
          warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "darkhound")
      {
        action_list_str = "travel/fel_bite";
        owner_coeff.ap_from_sp = 0.065;
      }

      void darkhound_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();

        melee_attack = new warlock_pet_melee_t(this, 2.0);
      }

      void darkhound_t::arise()
      {
        warlock_pet_t::arise();
        fel_bite->cooldown->start(timespan_t::from_millis(rng().range(3000, 5000)));
      }

      action_t* darkhound_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "fel_bite")
        {
          assert(fel_bite == nullptr);
          fel_bite = new fel_bite_t(this);
          return fel_bite;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace bilescourge {
      struct toxic_bile_t : public warlock_pet_spell_t
      {
        toxic_bile_t(warlock_pet_t* p) : warlock_pet_spell_t("toxic_bile", p, p -> find_spell(272167))
        {
        }
      };

      bilescourge_t::bilescourge_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "bilescourge") 
      {
        action_list_str = "toxic_bile";
        owner_coeff.ap_from_sp = 0.065;
      }

      void bilescourge_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
      }

      action_t* bilescourge_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "toxic_bile") return new toxic_bile_t(this);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace urzul {
      struct many_faced_bite_t : public warlock_pet_melee_attack_t
      {
        many_faced_bite_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("many_faced_bite", p, p -> find_spell(272439))
        {
          attack_power_mod.direct = data().effectN(1).ap_coeff();
        }

        void execute() override {
          warlock_pet_melee_attack_t::execute();
          cooldown->start(timespan_t::from_millis(rng().range(4500, 6000)));
        }
      };

      urzul_t::urzul_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "urzul") 
        ,many_faced_bite()
      {
        action_list_str = "travel";
        action_list_str += "/many_faced_bite";
        owner_coeff.ap_from_sp = 0.065;
      }

      void urzul_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        melee_attack = new warlock_pet_melee_t(this, 2.0);
      }

      void urzul_t::arise()
      {
        warlock_pet_t::arise();
        many_faced_bite->cooldown->start(timespan_t::from_millis(rng().range(3500, 4500)));
      }

      action_t* urzul_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "many_faced_bite")
        {
          assert(many_faced_bite == nullptr);
          many_faced_bite = new many_faced_bite_t(this);
          return many_faced_bite;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace void_terror {
      struct double_breath_damage_t : public warlock_pet_spell_t
      {
        double_breath_damage_t(warlock_pet_t* p, int breath_num) : warlock_pet_spell_t("double_breath-" + std::to_string(breath_num), p, p -> find_spell(272156))
        {
          attack_power_mod.direct = data().effectN(breath_num).ap_coeff();
        }
      };

      struct double_breath_t : public warlock_pet_spell_t
      {
        double_breath_damage_t* breath_1;
        double_breath_damage_t* breath_2;

        double_breath_t(warlock_pet_t* p) : warlock_pet_spell_t("double_breath", p, p -> find_spell(272156))
        {
          breath_1 = new double_breath_damage_t(p, 1);
          breath_2 = new double_breath_damage_t(p, 2);
          add_child(breath_1);
          add_child(breath_2);
        }

        void execute() override
        {
          cooldown->start(timespan_t::from_millis(rng().range(6000, 9000)));
          breath_1->execute();
          breath_2->execute();
        }
      };

      void_terror_t::void_terror_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "void_terror") 
       ,double_breath()
      {
        action_list_str = "travel";
        action_list_str += "/double_breath";
        owner_coeff.ap_from_sp = 0.065;
      }

      void void_terror_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        melee_attack = new warlock_pet_melee_t(this, 2.0);
      }

      void void_terror_t::arise()
      {
        warlock_pet_t::arise();
        double_breath->cooldown->start(timespan_t::from_millis(rng().range(1800, 5000)));
      }

      action_t* void_terror_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "double_breath")
        {
          assert(double_breath == nullptr);
          double_breath = new double_breath_t(this);
          return double_breath;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace wrathguard {
      struct overhead_assault_t : public warlock_pet_melee_attack_t
      {
        overhead_assault_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("overhead_assault", p, p -> find_spell(272432))
        {
          attack_power_mod.direct = data().effectN(1).ap_coeff();
        }

        void execute() override {
          warlock_pet_melee_attack_t::execute();
          cooldown->start(timespan_t::from_millis(rng().range(4500, 6500)));
        }
      };

      wrathguard_t::wrathguard_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "wrathguard") 
       ,overhead_assault()
      {
        action_list_str = "travel/overhead_assault";
        owner_coeff.ap_from_sp = 0.065;
      }

      void wrathguard_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        off_hand_weapon = main_hand_weapon;
        melee_attack = new warlock_pet_melee_t(this, 2.0);
      }

      void wrathguard_t::arise()
      {
        warlock_pet_t::arise();
        overhead_assault->cooldown->start(timespan_t::from_millis(rng().range(3000, 5000)));
      }

      action_t* wrathguard_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "overhead_assault")
        {
          assert(overhead_assault == nullptr);
          overhead_assault = new overhead_assault_t(this);
          return overhead_assault;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace vicious_hellhound {
      struct demon_fangs_t : public warlock_pet_melee_attack_t
      {
        demon_fangs_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("demon_fangs", p, p -> find_spell(272013))
        {
          attack_power_mod.direct = data().effectN(1).ap_coeff();
        }

        void execute() override {
          warlock_pet_melee_attack_t::execute();
          cooldown->start(timespan_t::from_millis(rng().range(4500, 6000)));
        }
      };

      vicious_hellhound_t::vicious_hellhound_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "vicious_hellhound"), demon_fang() 
      {
        action_list_str = "travel";
        action_list_str += "/demon_fangs";
        owner_coeff.ap_from_sp = 0.065;
      }

      void vicious_hellhound_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();

        main_hand_weapon.swing_time = timespan_t::from_seconds(1.0);
        melee_attack = new warlock_pet_melee_t(this, 1.0);
      }

      void vicious_hellhound_t::arise()
      {
        warlock_pet_t::arise();
        demon_fang->cooldown->start(timespan_t::from_millis(rng().range(3200, 5100)));
      }

      action_t* vicious_hellhound_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "demon_fangs")
        {
          assert(demon_fang == nullptr);
          demon_fang = new demon_fangs_t(this);
          return demon_fang;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace illidari_satyr {
      struct shadow_slash_t : public warlock_pet_melee_attack_t
      {
        shadow_slash_t(warlock_pet_t* p) : warlock_pet_melee_attack_t("shadow_slash", p, p -> find_spell(272012))
        {
          attack_power_mod.direct = data().effectN(1).ap_coeff();
        }

        void execute() override {
          warlock_pet_melee_attack_t::execute();
          cooldown->start(timespan_t::from_millis(rng().range(4500, 6100)));
        }
      };

      illidari_satyr_t::illidari_satyr_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "illidari_satyr") 
       ,shadow_slash()
      {
        action_list_str = "travel/shadow_slash";
        owner_coeff.ap_from_sp = 0.065;
      }

      void illidari_satyr_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        off_hand_weapon = main_hand_weapon;
        melee_attack = new warlock_pet_melee_t(this, 1.0);
      }

      void illidari_satyr_t::arise()
      {
        warlock_pet_t::arise();
        shadow_slash->cooldown->start(timespan_t::from_millis(rng().range(3500, 5000)));
      }

      action_t* illidari_satyr_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "shadow_slash")
        {
          assert(shadow_slash == nullptr);
          shadow_slash = new shadow_slash_t(this);
          return shadow_slash;
        }

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace eyes_of_guldan {
      struct eye_of_guldan_t : public warlock_pet_spell_t
      {
        eye_of_guldan_t(warlock_pet_t* p) : warlock_pet_spell_t("eye_of_guldan", p, p -> find_spell(272131))
        {
          hasted_ticks = false;
        }
      };

      eyes_of_guldan_t::eyes_of_guldan_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "eyes_of_guldan") {
        action_list_str = "eye_of_guldan";
        owner_coeff.ap_from_sp = 0.065;
      }

      void eyes_of_guldan_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
      }

      void eyes_of_guldan_t::arise()
      {
        warlock_pet_t::arise();
        o()->buffs.eyes_of_guldan->trigger();
      }

      void eyes_of_guldan_t::demise()
      {
        warlock_pet_t::demise();
        o()->buffs.eyes_of_guldan->decrement();
      }

      action_t* eyes_of_guldan_t::create_action(const std::string& name, const std::string& options_str) {
        if (name == "eye_of_guldan") return new eye_of_guldan_t(this);

        return warlock_pet_t::create_action(name, options_str);
      }
    }
    namespace prince_malchezaar {
      prince_malchezaar_t::prince_malchezaar_t(sim_t* sim, warlock_t* owner, const std::string& name) : warlock_pet_t(sim, owner, name, PET_WARLOCK_RANDOM, name != "prince_malchezaar") {
        owner_coeff.ap_from_sp = 0.616;
        action_list_str = "travel";
      }

      void prince_malchezaar_t::init_base_stats()
      {
        warlock_pet_t::init_base_stats();
        off_hand_weapon = main_hand_weapon;
        melee_attack = new warlock_pet_melee_t(this);
      }

      void prince_malchezaar_t::arise()
      {
        warlock_pet_t::arise();
        o()->buffs.prince_malchezaar->trigger();
      }

      void prince_malchezaar_t::demise()
      {
        warlock_pet_t::demise();
        o()->buffs.prince_malchezaar->decrement();
      }
    }

    void warlock_pet_t::create_buffs_demonology() {
      buffs.demonic_strength = make_buff(this, "demonic_strength", find_spell(267171))
        ->set_default_value(find_spell(267171)->effectN(2).percent())
        ->set_cooldown(timespan_t::zero());
      buffs.demonic_consumption = make_buff(this, "demonic_consumption", find_spell(267972))
        ->set_default_value(find_spell(267972)->effectN(1).percent())
        ->set_max_stack(100);
      buffs.grimoire_of_service = make_buff(this, "grimoire_of_service", find_spell(216187) );
    }

    void warlock_pet_t::init_spells_demonology() {
      active.soul_strike                = new felguard::soul_strike_t(this);
      active.bile_spit                  = new vilefiend::bile_spit_t(this);
    }
  }

  namespace actions_demonology {
    using namespace actions;

    struct shadow_bolt_t : public warlock_spell_t
    {
      shadow_bolt_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t(p, "Shadow Bolt", p->specialization())
      {
        parse_options(options_str);
        energize_type = ENERGIZE_ON_CAST;
        energize_resource = RESOURCE_SOUL_SHARD;
        energize_amount = 1;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        p()->buffs.demonic_calling->trigger();
      }

      double action_multiplier() const override
      {
        double m = warlock_spell_t::action_multiplier();

        if (p()->talents.sacrificed_souls->ok())
        {
          double pets = 0;
          for (auto& pet : p()->pet_list)
          {
            auto lock_pet = dynamic_cast<pets::warlock_pet_t*>(pet);

            if (lock_pet == nullptr)
              continue;
            if (lock_pet->is_sleeping())
              continue;

            pets++;
          }
          m *= 1.0 + ( p()->talents.sacrificed_souls->effectN(1).percent() * pets );
        }

        return m;
      }
    };

    struct hand_of_guldan_t : public warlock_spell_t {
          int shards_used;

          hand_of_guldan_t(warlock_t* p, const std::string& options_str) : warlock_spell_t(p, "Hand of Gul'dan") {
              parse_options(options_str);
              aoe = -1;
              shards_used = 0;

              parse_effect_data(p->find_spell(86040)->effectN(1));
          }

          timespan_t travel_time() const override {
              return timespan_t::from_millis(700);
          }

          bool ready() override {
            if (p()->resources.current[RESOURCE_SOUL_SHARD] == 0.0)
            {
              return false;
            }
            return warlock_spell_t::ready();
          }

          double action_multiplier() const override
          {
            double m = warlock_spell_t::action_multiplier();

            m *= last_resource_cost;

            return m;
          }

          void consume_resource() override {
              warlock_spell_t::consume_resource();

              shards_used = as<int>(last_resource_cost);

              if (last_resource_cost == 1.0)
                  p()->procs.one_shard_hog->occur();
              if (last_resource_cost == 2.0)
                  p()->procs.two_shard_hog->occur();
              if (last_resource_cost == 3.0)
                  p()->procs.three_shard_hog->occur();
          }

          void impact(action_state_t* s) override {
              warlock_spell_t::impact(s);

              if (result_is_hit(s->result) & td(s->target) == td(target)) {
                int j = 0;

                for (auto imp : p()->warlock_pet_list.wild_imps)
                {
                  if (imp->is_sleeping())
                  {
                    imp->summon(timespan_t::from_seconds(25));
                    if (++j == shards_used) break;
                  }
                }

                if (p()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T21, B2)) {
                    for (int i = 0; i < shards_used; i++) {
                        p()->buffs.rage_of_guldan->trigger();
                    }
                }
              }
          }
      };

    struct demonbolt_t : public warlock_spell_t {
      demonbolt_t(warlock_t* p, const std::string& options_str) : warlock_spell_t(p, "Demonbolt") {
        parse_options(options_str);
        energize_type = ENERGIZE_ON_CAST;
        energize_resource = RESOURCE_SOUL_SHARD;
        energize_amount = 2;
      }

      timespan_t execute_time() const override
      {
        auto et = warlock_spell_t::execute_time();

        if ( p()->buffs.demonic_core->check() )
        {
          et *= 1.0 + p()->buffs.demonic_core -> data().effectN(1).percent();
        }

        return et;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        p()->buffs.demonic_core->up(); // benefit tracking
        p()->buffs.demonic_core->decrement();
        p()->buffs.demonic_calling->trigger();
      }

      double action_multiplier() const override
      {
        double m = warlock_spell_t::action_multiplier();

        if (p()->talents.sacrificed_souls->ok())
        {
          double pets = 0;
          for (auto& pet : p()->pet_list)
          {
            auto lock_pet = dynamic_cast<pets::warlock_pet_t*>(pet);

            if (lock_pet == nullptr)
              continue;
            if (lock_pet->is_sleeping())
              continue;

            pets++;
          }
          m *= 1.0 + (p()->talents.sacrificed_souls->effectN(1).percent() * pets);
        }

        return m;
      }
    };

    struct call_dreadstalkers_t : public warlock_spell_t {
      timespan_t dreadstalker_duration;
      int dreadstalker_count;

      call_dreadstalkers_t(warlock_t* p, const std::string& options_str) : warlock_spell_t(p, "Call Dreadstalkers") {
        parse_options(options_str);
        may_crit = false;
        dreadstalker_duration = p->find_spell(193332)->duration() + (p->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T19, B4) ? p->sets->set(WARLOCK_DEMONOLOGY, T19, B4)->effectN(1).time_value() : timespan_t::zero());
        dreadstalker_count = data().effectN(1).base_value();
      }

      double cost() const override
      {
        double c = warlock_spell_t::cost();
        if (p()->buffs.demonic_calling->check())
        {
          c -= p()->talents.demonic_calling->effectN(1).base_value();
        }

        return  c;
      }

      timespan_t execute_time() const override
      {
        if (p()->buffs.demonic_calling->check())
        {
          return timespan_t::zero();
        }

        return warlock_spell_t::execute_time();
      }

      void execute() override
      {
        warlock_spell_t::execute();

        int j = 0;

        for (size_t i = 0; i < p()->warlock_pet_list.dreadstalkers.size(); i++)
        {
          if (p() -> warlock_pet_list.dreadstalkers[i]->is_sleeping())
          {
            p()->warlock_pet_list.dreadstalkers[i]->summon(dreadstalker_duration);
            p()->procs.dreadstalker_debug->occur();

            if (p()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T21, B2))
            {
              p()->warlock_pet_list.dreadstalkers[i]->buffs.rage_of_guldan->set_duration(dreadstalker_duration);
              p()->warlock_pet_list.dreadstalkers[i]->buffs.rage_of_guldan->set_default_value(p()->buffs.rage_of_guldan->stack_value());
              p()->warlock_pet_list.dreadstalkers[i]->buffs.rage_of_guldan->trigger();
            }
              
            /*
            if (p()->legendary.wilfreds_sigil_of_superior_summoning_flag && !p()->talents.grimoire_of_supremacy->ok())
            {
              p()->cooldowns.doomguard->adjust(p()->legendary.wilfreds_sigil_of_superior_summoning);
              p()->cooldowns.infernal->adjust(p()->legendary.wilfreds_sigil_of_superior_summoning);
              p()->procs.wilfreds_dog->occur();
            }
            */
            if (++j == dreadstalker_count) break;
          }
        }

        p()->buffs.demonic_calling->up(); // benefit tracking
        p()->buffs.demonic_calling->decrement();
        p()->buffs.rage_of_guldan->expire();

        if (p()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T20, B4))
        {
          p()->buffs.dreaded_haste->trigger();
        }

        if (p()->talents.from_the_shadows->ok())
        {
          td(target)->debuffs_from_the_shadows->trigger();
        }
      }
    };

    struct implosion_t : public warlock_spell_t
    {
      struct implosion_aoe_t : public warlock_spell_t
      {
        double casts_left = 5.0;

        implosion_aoe_t(warlock_t* p) :
          warlock_spell_t("implosion_aoe", p, p -> find_spell(196278))
        {
          aoe = -1;
          dual = true;
          background = true;
          callbacks = false;

          p->spells.implosion_aoe = this;
        }

        double composite_target_multiplier(player_t* t) const override
        {
          double m = warlock_spell_t::composite_target_multiplier(t);

          if (t == this->target)
          {
            m *= (casts_left / 5.0);
          }
          
          return m;
        }
      };

      implosion_aoe_t* explosion;

      implosion_t(warlock_t* p, const std::string& options_str) : warlock_spell_t("implosion", p),explosion(new implosion_aoe_t(p))
      {
        parse_options(options_str);
        aoe = -1;
        add_child(explosion);
      }

      bool ready() override
      {
        bool r = warlock_spell_t::ready();

        if (r)
        {
          for (auto imp : p()->warlock_pet_list.wild_imps)
          {
            if (!imp->is_sleeping())
              return true;
          }
        }
        return false;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        for (auto imp : p()->warlock_pet_list.wild_imps)
        {
          if (!imp->is_sleeping())
          {
            explosion->casts_left = (imp->resources.current[RESOURCE_ENERGY] / 20);
            explosion->set_target(this->target);
            explosion->execute();
            imp->dismiss();
          }
        }
      }
    };

    struct summon_demonic_tyrant_t : public warlock_spell_t
    {
      summon_demonic_tyrant_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("summon_demonic_tyrant", p, p -> find_spell(265187))
      {
        parse_options(options_str);
        harmful = may_crit = false;
      }

      void execute() override
      {
        warlock_spell_t::execute();

        for ( auto& demonic_tyrant : p()->warlock_pet_list.demonic_tyrants)
        {
          if (demonic_tyrant->is_sleeping())
          {
            demonic_tyrant->summon(data().duration());
          }
        }

        p()->buffs.demonic_power->trigger();

        if (p()->talents.demonic_consumption->ok())
        {
          for (auto imp : p()->warlock_pet_list.wild_imps)
          {
            if (!imp->is_sleeping())
            {
              double available = imp->resources.current[RESOURCE_ENERGY];
              imp->dismiss(true);
              for (auto dt : p()->warlock_pet_list.demonic_tyrants)
              {
                if (!dt->is_sleeping())
                {
                  for (int i = 0; i < (available/20*3); i++) // TODO: check if hardcoded value can be replaced.
                  {
                    dt->buffs.demonic_consumption->trigger();
                  }
                }
              }
            }
          }
        }

        for (auto& pet : p()->pet_list)
        {
          auto lock_pet = dynamic_cast<pets::warlock_pet_t*>(pet);

          if (lock_pet == nullptr)
            continue;
          if (lock_pet->is_sleeping())
            continue;

          if (lock_pet->pet_type == PET_DEMONIC_TYRANT)
            continue;

          if (lock_pet->expiration)
          {
            timespan_t new_time = lock_pet->expiration->time + p()->buffs.demonic_power->data().effectN(3).time_value();
            lock_pet->expiration->reschedule_time = new_time;
          }
        }

        p()->buffs.tyrant->set_duration(data().duration());
        p()->buffs.tyrant->trigger();
        if (p()->buffs.dreadstalkers->check())
        {
          p()->buffs.dreadstalkers->extend_duration(p(), p()->buffs.demonic_power->data().effectN(3).time_value());
        }
        if (p()->buffs.grimoire_felguard->check())
        {
          p()->buffs.grimoire_felguard->extend_duration(p(), p()->buffs.demonic_power->data().effectN(3).time_value());
        }
        if (p()->buffs.vilefiend->check())
        {
          p()->buffs.vilefiend->extend_duration(p(), p()->buffs.demonic_power->data().effectN(3).time_value());
        }
      }
    };

    // Talents
    struct demonic_strength_t : public warlock_spell_t
    {
      demonic_strength_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("demonic_strength", p, p->talents.demonic_strength)
      {
        parse_options(options_str);
      }

      bool ready() override
      {
        if (p()->warlock_pet_list.active->pet_type != PET_FELGUARD)
          return false;
        if (p()->get_dot("felstorm", p()->warlock_pet_list.active)->is_ticking())
          return false;
        if (p()->get_dot("demonic_strength_felstorm", p()->warlock_pet_list.active)->is_ticking())
          return false;

        return spell_t::ready();
      }

      void execute() override
      {
        warlock_spell_t::execute();
        if (p()->warlock_pet_list.active->pet_type == PET_FELGUARD)
        {
          p()->warlock_pet_list.active->buffs.demonic_strength->trigger();

          assert( p()->warlock_pet_list.active->active.demonic_strength_felstorm );
          auto ds = debug_cast<pets::felguard::demonic_strength_t*>(p()->warlock_pet_list.active->active.demonic_strength_felstorm);
          ds->queued = true;
        }
      }
    };

    struct bilescourge_bombers_t : public warlock_spell_t
    {
      struct bilescourge_bombers_tick_t : public warlock_spell_t
      {
        bilescourge_bombers_tick_t(warlock_t* p) :
          warlock_spell_t("bilescourge_bombers_tick", p, p -> find_spell(267213))
        {
          aoe = -1;
          background = dual = direct_tick = true;
          callbacks = false;
          radius = p->talents.bilescourge_bombers->effectN(1).radius();
        }
      };

      bilescourge_bombers_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("bilescourge_bombers", p, p->talents.bilescourge_bombers)
      {
        parse_options(options_str);
        dot_duration = timespan_t::zero();
        may_miss = may_crit = false;
        base_tick_time = data().duration() / 12.0;
        base_execute_time = timespan_t::zero();

        if (!p->active.bilescourge_bombers)
        {
          p->active.bilescourge_bombers = new bilescourge_bombers_tick_t(p);
          p->active.bilescourge_bombers->stats = stats;
        }
      }

      void execute() override
      {
        warlock_spell_t::execute();

        make_event<ground_aoe_event_t>(*sim, p(), ground_aoe_params_t()
          .target(execute_state->target)
          .x(execute_state->target->x_position)
          .y(execute_state->target->y_position)
          .pulse_time(base_tick_time * player->cache.spell_haste())
          .duration(data().duration() * player->cache.spell_haste())
          .start_time(sim->current_time())
          .action(p()->active.bilescourge_bombers));
      }
    };

    struct power_siphon_t : public warlock_spell_t {
      power_siphon_t(warlock_t* p, const std::string& options_str) : warlock_spell_t("power_siphon", p, p -> talents.power_siphon) {
        parse_options(options_str);
        harmful = false;
        ignore_false_positive = true;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        
        struct lower_energy
        {
          inline bool operator() (const pets::wild_imp::wild_imp_pet_t* imp1, const pets::wild_imp::wild_imp_pet_t* imp2)
          {
            return (imp1->resources.current[RESOURCE_ENERGY] > imp2->resources.current[RESOURCE_ENERGY]);
          }
        };

        std::vector<pets::wild_imp::wild_imp_pet_t*> imps;

        for (auto imp : p()->warlock_pet_list.wild_imps)
        {
          if (!imp->is_sleeping())
          {
            imps.push_back(imp);
          }
        }
        
        std::sort(imps.begin(), imps.end(), lower_energy());
        if(imps.size()>p()->talents.power_siphon->effectN(1).base_value()) imps.resize(p()->talents.power_siphon->effectN(1).base_value());

        while (imps.size() > 0)
        {
          p()->buffs.demonic_core->trigger();
          pets::wild_imp::wild_imp_pet_t* imp = imps.front();
          imps.erase(imps.begin());
          imp->dismiss(true);
        }
      }
    };

    struct doom_t : public warlock_spell_t {
      doom_t(warlock_t* p, const std::string& options_str) : warlock_spell_t("doom", p, p -> talents.doom) {
          parse_options(options_str);

          base_tick_time = data().duration();
          dot_duration = data().duration();
          spell_power_mod.tick = p->find_spell(265469)->effectN(1).sp_coeff();

          energize_type = ENERGIZE_PER_TICK;
          energize_resource = RESOURCE_SOUL_SHARD;
          energize_amount = 1;

          may_crit = true;
          hasted_ticks = true;
      }

      timespan_t composite_dot_duration(const action_state_t* s) const override {
          timespan_t duration = warlock_spell_t::composite_dot_duration(s);
          return duration * p()->cache.spell_haste();
      }

      void tick(dot_t* d) override {
          warlock_spell_t::tick(d);
          if (d->state->result == RESULT_HIT || result_is_hit(d->state->result)) {
              if (p()->sets->has_set_bonus(WARLOCK_DEMONOLOGY, T19, B2) && rng().roll(p()->sets->set(WARLOCK_DEMONOLOGY, T19, B2)->effectN(1).percent()))
                  p()->resource_gain(RESOURCE_SOUL_SHARD, 1, p()->gains.t19_2pc_demonology);
          }
      }
    };

    struct soul_strike_t : public warlock_spell_t
    {
      soul_strike_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("Soul Strike", p, p->talents.soul_strike)
      {
        parse_options(options_str);
        energize_type = ENERGIZE_ON_CAST;
        energize_resource = RESOURCE_SOUL_SHARD;
        energize_amount = 1;
      }
      void execute() override
      {
        warlock_spell_t::execute();
        if (p()->warlock_pet_list.active->pet_type == PET_FELGUARD)
        {
          p()->warlock_pet_list.active->active.soul_strike->set_target(execute_state->target);
          p()->warlock_pet_list.active->active.soul_strike->execute();
        }
      }
      bool ready() override
      {
        if (p()->warlock_pet_list.active->pet_type == PET_FELGUARD)
          return warlock_spell_t::ready();

        return false;
      }
    };

    struct summon_vilefiend_t : public warlock_spell_t
    {
      summon_vilefiend_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("summon_vilefiend", p, p->talents.summon_vilefiend)
      {
        parse_options(options_str);
        harmful = may_crit = false;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        p()->buffs.vilefiend->set_duration(data().duration());
        p()->buffs.vilefiend->trigger();

        for (auto& vilefiend : p()->warlock_pet_list.vilefiends)
        {
          if (vilefiend->is_sleeping())
          {
            vilefiend->summon(data().duration());
            vilefiend->active.bile_spit->set_target(execute_state->target);
            vilefiend->active.bile_spit->execute();
          }
        }
      }
    };

    struct grimoire_felguard_t : public summon_pet_t {
      grimoire_felguard_t(warlock_t* p, const std::string& options_str) :
        summon_pet_t("grimoire_felguard", p, p -> talents.grimoire_felguard) {
        parse_options(options_str);
        cooldown->duration = data().cooldown();
        summoning_duration = data().duration() + timespan_t::from_millis(1); // TODO: why?
      }

      void execute() override {
          summon_pet_t::execute();
          pet->buffs.grimoire_of_service->trigger();
          p()->buffs.grimoire_felguard->set_duration(timespan_t::from_seconds(p()->talents.grimoire_felguard->effectN(1).base_value()));
          p()->buffs.grimoire_felguard->trigger();
      }
      bool init_finished() override {
          if (pet) {
              pet->summon_stats = stats;
          }
          return summon_pet_t::init_finished();
      }
      };

    struct inner_demons_t : public warlock_spell_t
    {
      inner_demons_t(warlock_t* p, const std::string& options_str) : warlock_spell_t("inner_demons", p)
      {
        parse_options(options_str);
        trigger_gcd = timespan_t::zero();
        harmful = false;
        ignore_false_positive = true;
        action_skill = 1;
      }

      void execute() override
      {
        warlock_spell_t::execute();
        p()->buffs.inner_demons->trigger();
      }
    };

    struct nether_portal_t : public warlock_spell_t
    {
      nether_portal_t(warlock_t* p, const std::string& options_str) :
        warlock_spell_t("nether_portal", p, p->talents.nether_portal)
      {
        parse_options(options_str);
        harmful = may_crit = may_miss = false;
      }

      void execute() override
      {
        p()->buffs.nether_portal->trigger();
        warlock_spell_t::execute();
      }
    };

    struct summon_random_demon_t : public warlock_spell_t {
      summon_random_demon_t(warlock_t* p, const std::string& options_str) : warlock_spell_t("summon_random_demon", p) {
        parse_options(options_str);
        background = true;
      }

      void execute() override
      {
        int demon_int = rng().range(10) + 1;
        int rare_check;
        if (demon_int <= 2) 
        {
          rare_check = rng().range(10) + 1;
          if (rare_check > 1)
          {
            demon_int = rng().range(8) + 3;
          }
        }
        int summon_time = 15;

        switch (demon_int) {
          case 1 : {
              for (auto demon : p()->warlock_pet_list.prince_malchezaar)
              {
                if (demon->is_sleeping())
                {
                  demon->summon(timespan_t::from_seconds(summon_time));
                  break;
                }
              }
              break;
          }
          case 2 : {
            for (auto demon : p()->warlock_pet_list.eyes_of_guldan)
            {
              for (int x = 0; x < 4; x++)
              {
                if (demon->is_sleeping())
                {
                  demon->summon(timespan_t::from_seconds(summon_time));
                  break;
                }
              }
            }
            break;
          }
          case 3 : {
            for (auto demon : p()->warlock_pet_list.shivarra)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 4 : {
            for (auto demon : p()->warlock_pet_list.darkhounds)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 5 : {
            for (auto demon : p()->warlock_pet_list.bilescourges)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 6 : {
            for (auto demon : p()->warlock_pet_list.urzuls)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 7 : {
            for (auto demon : p()->warlock_pet_list.void_terrors)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 8 : {
            for (auto demon : p()->warlock_pet_list.wrathguards)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 9 : {
            for (auto demon : p()->warlock_pet_list.vicious_hellhounds)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
          case 10 : {
            for (auto demon : p()->warlock_pet_list.illidari_satyrs)
            {
              if (demon->is_sleeping())
              {
                demon->summon(timespan_t::from_seconds(summon_time));
                break;
              }
            }
            break;
          }
        }
      }
    };

  } // end actions namespace
  namespace buffs {
  } // end buffs namespace

  // add actions
  action_t* warlock_t::create_action_demonology(const std::string& action_name, const std::string& options_str) {
    using namespace actions_demonology;

    if (action_name == "shadow_bolt") return new            shadow_bolt_t(this, options_str);
    if (action_name == "demonbolt") return new              demonbolt_t(this, options_str);
    if (action_name == "hand_of_guldan") return new         hand_of_guldan_t(this, options_str);
    if (action_name == "implosion") return new              implosion_t(this, options_str);

    if (action_name == "demonic_strength") return new       demonic_strength_t(this, options_str);
    if (action_name == "bilescourge_bombers") return new    bilescourge_bombers_t(this, options_str);
    if (action_name == "doom")          return new          doom_t(this, options_str);
    if (action_name == "power_siphon") return new           power_siphon_t(this, options_str);
    if (action_name == "soul_strike") return new            soul_strike_t(this, options_str);
    if (action_name == "inner_demons") return new           inner_demons_t(this, options_str);
    if (action_name == "nether_portal") return new          nether_portal_t(this, options_str);

    if (action_name == "call_dreadstalkers") return new     call_dreadstalkers_t(this, options_str);
    if (action_name == "summon_felguard") return new        summon_main_pet_t("felguard", this);
    if (action_name == "summon_demonic_tyrant") return new  summon_demonic_tyrant_t(this, options_str);
    if (action_name == "summon_vilefiend") return new       summon_vilefiend_t(this, options_str);
    if (action_name == "grimoire_felguard") return new      grimoire_felguard_t(this, options_str);

    return nullptr;
  }

  void warlock_t::create_buffs_demonology() {
    buffs.demonic_core = make_buff(this, "demonic_core", find_spell(264173));
    buffs.demonic_power = make_buff(this, "demonic_power", find_spell(265273))
      ->set_default_value(find_spell(265273)->effectN(2).percent())
      ->set_cooldown(timespan_t::zero());
    //Talents
    buffs.demonic_calling = make_buff(this, "demonic_calling", talents.demonic_calling->effectN(1).trigger())
      ->set_trigger_spell(talents.demonic_calling);
    buffs.inner_demons = make_buff(this, "inner_demons", find_spell(267216))
      ->set_period(timespan_t::from_seconds(talents.inner_demons->effectN(1).base_value()))
      ->set_tick_time_behavior(buff_tick_time_behavior::UNHASTED)
      ->set_tick_callback([this](buff_t*, int, const timespan_t&)
      {
        for (auto imp : warlock_pet_list.wild_imps)
        {
          if (imp->is_sleeping())
          {
            imp->summon();
            break;
          }
        }
        if (rng().roll(talents.inner_demons->effectN(1).percent())) {
          active.summon_random_demon->execute();
        }
      });
    buffs.nether_portal = make_buff(this, "nether_portal", talents.nether_portal)
      ->set_duration(talents.nether_portal->duration());
    //Tier
    buffs.rage_of_guldan = make_buff(this, "rage_of_guldan", sets->set(WARLOCK_DEMONOLOGY, T21, B2)->effectN(1).trigger())
      ->set_duration(find_spell(257926)->duration())
      ->set_max_stack(find_spell(257926)->max_stacks())
      ->set_default_value(find_spell(257926)->effectN(1).base_value())
      ->set_refresh_behavior(buff_refresh_behavior::DURATION);
    buffs.dreaded_haste = make_buff<haste_buff_t>(this, "dreaded_haste", sets->set(WARLOCK_DEMONOLOGY, T20, B4)->effectN(1).trigger())
      ->set_default_value(sets->set(WARLOCK_DEMONOLOGY, T20, B4)->effectN(1).trigger()->effectN(1).percent());

    //to track imps
    buffs.wild_imps = make_buff(this, "wild_imps")
      ->set_max_stack(40);
    buffs.dreadstalkers = make_buff(this, "dreadstalkers")
      ->set_max_stack(4);
    buffs.vilefiend = make_buff(this, "vilefiend")
      ->set_max_stack(1);
    buffs.tyrant = make_buff(this, "tyrant")
      ->set_max_stack(1);
    buffs.grimoire_felguard = make_buff(this, "grimoire_felguard")
      ->set_max_stack(1);
    buffs.prince_malchezaar = make_buff(this, "prince_malchezaar")
      ->set_max_stack(1);
    buffs.eyes_of_guldan = make_buff(this, "eyes_of_guldan")
      ->set_max_stack(4);
    buffs.portal_summons = make_buff(this, "portal_summons")
      ->set_duration(timespan_t::from_seconds(15))
      ->set_max_stack(40)
      ->set_refresh_behavior(buff_refresh_behavior::DURATION);
  }

  void warlock_t::init_spells_demonology() {
    spec.demonology                         = find_specialization_spell(137044);
    mastery_spells.master_demonologist      = find_mastery_spell(WARLOCK_DEMONOLOGY);
    // spells
    // Talents
    talents.dreadlash                       = find_talent_spell("Dreadlash");
    talents.demonic_strength                = find_talent_spell("Demonic Strength");
    talents.bilescourge_bombers             = find_talent_spell("Bilescourge Bombers");
    talents.demonic_calling                 = find_talent_spell("Demonic Calling");
    talents.power_siphon                    = find_talent_spell("Power Siphon");
    talents.doom                            = find_talent_spell("Doom");
    talents.from_the_shadows                = find_talent_spell("From the Shadows");
    talents.soul_strike                     = find_talent_spell("Soul Strike");
    talents.summon_vilefiend                = find_talent_spell("Summon Vilefiend");
    talents.inner_demons                    = find_talent_spell("Inner Demons");
    talents.grimoire_felguard               = find_talent_spell("Grimoire: Felguard");
    talents.sacrificed_souls                = find_talent_spell("Sacrificed Souls");
    talents.demonic_consumption             = find_talent_spell("Demonic Consumption");
    talents.nether_portal                   = find_talent_spell("Nether Portal");

    active.summon_random_demon              = new actions_demonology::summon_random_demon_t(this, "");
  }

  void warlock_t::init_gains_demonology() {
    gains.t19_2pc_demonology = get_gain("t19_2pc_demonology");
  }

  void warlock_t::init_rng_demonology() {
  }

  void warlock_t::init_procs_demonology() {
    procs.dreadstalker_debug = get_proc("dreadstalker_debug");
  }

  void warlock_t::create_options_demonology() {
  }

  void warlock_t::create_apl_demonology() {
    action_priority_list_t* def = get_action_priority_list("default");
    action_priority_list_t* np = get_action_priority_list("nether_portal");
    action_priority_list_t* npb = get_action_priority_list("nether_portal_building");
    action_priority_list_t* npa = get_action_priority_list("nether_portal_active");
    action_priority_list_t* bas = get_action_priority_list("build_a_shard");
    
    def->add_action("demonic_strength");
    def->add_action("call_action_list,name=nether_portal,if=talent.nether_portal.enabled");

    np->add_action("call_action_list,name=nether_portal_building,if=cooldown.nether_portal.remains<20");
    np->add_action("call_action_list,name=nether_portal_active,if=cooldown.nether_portal.remains>160");

    npb->add_action("nether_portal,if=soul_shard>=5&(!talent.power_siphon.enabled|buff.demonic_core.up)");
    npb->add_action("call_dreadstalkers");
    npb->add_action("hand_of_guldan,if=cooldown.call_dreadstalkers.remains>18&soul_shard>=3");
    npb->add_action("power_siphon,if=buff.wild_imps.stack>=2&buff.demonic_core.stack<=2&buff.demonic_power.down&soul_shard>=3");
    npb->add_action("hand_of_guldan,if=soul_shard>=5");
    npb->add_action("call_action_list,name=build_a_shard");

    npa->add_action("call_dreadstalkers");
    npa->add_action("hand_of_guldan,if=prev_gcd.1.grimoire_felguard|prev_gcd.1.summon_vilefiend");
    npa->add_action("grimoire_felguard");
    npa->add_action("summon_vilefiend");
    npa->add_action("bilescourge_bombers");
    npa->add_action("call_action_list,name=build_a_shard,if=soul_shard=1&(cooldown.call_dreadstalkers.remains<action.shadow_bolt.cast_time|(talent.bilescourge_bombers.enabled&cooldown.bilescourge_bombers.remains<action.shadow_bolt.cast_time))");
    npa->add_action("hand_of_guldan,if=((cooldown.call_dreadstalkers.remains>action.demonbolt.cast_time)&(cooldown.call_dreadstalkers.remains>action.shadow_bolt.cast_time))&cooldown.nether_portal.remains>(160+action.hand_of_guldan.cast_time)");
    npa->add_action("summon_demonic_tyrant,if=buff.nether_portal.remains<10&soul_shard=0");
    npa->add_action("summon_demonic_tyrant,if=buff.nether_portal.remains<action.summon_demonic_tyrant.cast_time+5.5");
    npa->add_action("demonbolt,if=buff.demonic_core.up");
    npa->add_action("call_action_list,name=build_a_shard");

    bas->add_action("soul_strike");
    bas->add_action("shadow_bolt");

    def->add_action("summon_vilefiend,if=cooldown.summon_demonic_tyrant.remains>30|(cooldown.summon_demonic_tyrant.remains<10&cooldown.call_dreadstalkers.remains<10)");
    def->add_action("grimoire_felguard");
    def->add_action("hand_of_guldan,if=soul_shard>=5");
    def->add_action("hand_of_guldan,if=soul_shard>=3&cooldown.call_dreadstalkers.remains>4&(!talent.summon_vilefiend.enabled|cooldown.summon_vilefiend.remains>4)");
    def->add_action("call_dreadstalkers");
    def->add_action("bilescourge_bombers");
    def->add_action("summon_demonic_tyrant,if=talent.summon_vilefiend.enabled&buff.dreadstalkers.remains>action.summon_demonic_tyrant.cast_time&buff.vilefiend.remains>action.summon_demonic_tyrant.cast_time");
    def->add_action("summon_demonic_tyrant,if=!talent.summon_vilefiend.enabled&buff.dreadstalkers.remains>action.summon_demonic_tyrant.cast_time&soul_shard=0");
    def->add_action("power_siphon,if=buff.wild_imps.stack>=2&buff.demonic_core.stack<=2&buff.demonic_power.down");
    def->add_action("demonbolt,if=soul_shard<=3&buff.demonic_core.up");
    def->add_action("doom,cycle_targets=1,if=(talent.doom.enabled&target.time_to_die>duration&(!ticking|remains<duration*0.3))");
    def->add_action("call_action_list,name=build_a_shard");
  }

  using namespace unique_gear;
  using namespace actions;

  void warlock_t::legendaries_demonology() {

  }
}
