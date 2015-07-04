// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#ifndef SIMULATIONCRAFT_H
static_assert( 0 , "dbc.hpp included into a file where SIMULATIONCRAFT_H is not defined!" );
/* This Header cannot stand on its own feet.
 * It only works when included into simulationcraft.hpp at a specific place.
 * The purpose (for now) is only to have it sourced out into a separate file.
 */
#endif

#ifndef SC_DBC_HPP
#define SC_DBC_HPP

#include "data_definitions.hh"
#include "data_enums.hh"

// This is a automatically generated header.
#include "specialization.hpp"

static const unsigned NUM_SPELL_FLAGS = 12;
static const unsigned NUM_CLASS_FAMILY_FLAGS = 4;

// ==========================================================================
// General Database
// ==========================================================================

namespace dbc {
// Wrapper for fetching spell data through various spell data variants
const spell_data_t* find_spell( const player_t*, const spell_data_t* spell );
const spell_data_t* find_spell( const player_t*, unsigned spell_id );

// Initialization
void apply_hotfixes();
void init();
void init_item_data();
void de_init();

// Utily functions
uint32_t get_school_mask( school_e s );
school_e get_school_type( uint32_t school_id );
bool is_school( school_e s, school_e s2 );
unsigned specialization_max_per_class();
specialization_e spec_by_idx( const player_e c, unsigned idx );

// Data Access
int build_level( bool ptr );
const char* wow_version( bool ptr );
const char* wow_ptr_status( bool ptr );
const item_data_t* items( bool ptr );
item_data_t* __items_noptr();
item_data_t* __items_ptr();
std::size_t        n_items( bool ptr );
std::size_t        n_items_noptr();
std::size_t        n_items_ptr();
const item_set_bonus_t* set_bonus( bool ptr );
std::size_t             n_set_bonus( bool ptr );
const item_enchantment_data_t* item_enchantments( bool ptr );
std::size_t        n_item_enchantments( bool ptr );
const gem_property_data_t* gem_properties( bool ptr );
specialization_e translate_spec_str   ( player_e ptype, const std::string& spec_str );
std::string specialization_string     ( specialization_e spec );
double fmt_value( double v, effect_type_t type, effect_subtype_t sub_type );
const std::string& get_token( unsigned int id_spell );
bool add_token( unsigned int id_spell, const std::string& token_name, bool ptr );
unsigned int get_token_id( const std::string& token );
bool valid_gem_color( unsigned color );

// Filtered data access
const item_data_t* find_potion( bool ptr, const std::function<bool(const item_data_t*)>& finder );
}

namespace hotfix
{
  enum hotfix_op_e
  {
    HOTFIX_NONE = 0,
    HOTFIX_SET,
    HOTFIX_ADD,
    HOTFIX_MUL,
    HOTFIX_DIV
  };

  enum hotfix_flags_e
  {
    HOTFIX_FLAG_LIVE  = 0x1,
    HOTFIX_FLAG_PTR   = 0x2,
    HOTFIX_FLAG_QUIET = 0x4,

    HOTFIX_FLAG_DEFAULT = HOTFIX_FLAG_LIVE | HOTFIX_FLAG_PTR
  };

  struct custom_dbc_data_t
  {
    auto_dispose< std::vector<spell_data_t*> > spells_[ 2 ];
    auto_dispose< std::vector<spelleffect_data_t*> > effects_[ 2 ];

    bool add_spell( spell_data_t* spell, bool ptr = false );
    spell_data_t* get_mutable_spell( unsigned spell_id, bool ptr = false );
    const spell_data_t* find_spell( unsigned spell_id, bool ptr = false ) const;

    bool add_effect( spelleffect_data_t* spell, bool ptr = false );
    spelleffect_data_t* get_mutable_effect( unsigned effect_id, bool ptr = false );
    const spelleffect_data_t* find_effect( unsigned effect_id, bool ptr = false ) const;

    // Creates a tree of cloned spells and effects given a spell id, starting from the potential
    // root spell. If there is no need to clone the tree, return the custom spell instead.
    spell_data_t* clone_spell( unsigned spell_id, bool ptr = false );

    private:
    spell_data_t* create_clone( const spell_data_t* s, bool ptr );
  };

  struct hotfix_entry_t
  {
    std::string tag_;
    std::string note_;
    unsigned    flags_;

    hotfix_entry_t() :
      tag_(), note_(), flags_( 0 )
    { }

    hotfix_entry_t( const std::string& t, const std::string& n, unsigned f ) :
      tag_( t ), note_( n ), flags_( f )
    { }

    virtual ~hotfix_entry_t() { }

    virtual void apply() { }
    virtual std::string to_str() const;
  };

  struct dbc_hotfix_entry_t : public hotfix_entry_t
  {
    unsigned      id_;    // Spell ID
    std::string   field_name_;  // Field name to override
    hotfix_op_e   operation_;   // Operation to perform on the current DBC value
    double        modifier_;    // Modifier to apply to the operation
    double        orig_value_;  // The value to check against for DBC data changes
    double        dbc_value_;   // The value in the DBC before applying hotfix
    double        hotfix_value_; // The DBC value after hotfix applied

    dbc_hotfix_entry_t() :
      hotfix_entry_t(), id_( 0 ), field_name_(), operation_( HOTFIX_NONE ), modifier_( 0 ),
      orig_value_( -std::numeric_limits<double>::max() ), dbc_value_( 0 ), hotfix_value_( 0 )
    { }

    dbc_hotfix_entry_t( const std::string& t, unsigned id, const std::string& n, unsigned f ) :
      hotfix_entry_t( t, n, f ),
      id_( id ), field_name_(), operation_( HOTFIX_NONE ), modifier_( 0 ),
      orig_value_( -std::numeric_limits<double>::max() ), dbc_value_( 0 ), hotfix_value_( 0 )
    { }

    virtual void apply()
    {
      if ( flags_ & HOTFIX_FLAG_LIVE )
      {
        apply_hotfix( false );
      }

#if SC_USE_PTR
      if ( flags_ & HOTFIX_FLAG_PTR )
      {
        apply_hotfix( true );
      }
#endif
    }

    dbc_hotfix_entry_t& field( const std::string& fn )
    { field_name_ = fn; return *this; }

    dbc_hotfix_entry_t& operation( hotfix::hotfix_op_e op )
    { operation_ = op; return *this; }

    dbc_hotfix_entry_t& modifier( double m )
    { modifier_ = m; return *this; }

    dbc_hotfix_entry_t& verification_value( double ov )
    { orig_value_ = ov; return *this; }

    private:
    virtual void apply_hotfix( bool ptr = false ) = 0;
  };

  struct spell_hotfix_entry_t : public dbc_hotfix_entry_t
  {
    spell_hotfix_entry_t( const std::string& t, unsigned id, const std::string& n, unsigned f ) :
      dbc_hotfix_entry_t( t, id, n, f )
    { }

    std::string to_str() const;

    private:
    void apply_hotfix( bool ptr );
  };

  struct effect_hotfix_entry_t : public dbc_hotfix_entry_t
  {
    effect_hotfix_entry_t( const std::string& t, unsigned id, const std::string& n, unsigned f ) :
      dbc_hotfix_entry_t( t, id, n, f )
    { }

    std::string to_str() const;

    private:
    void apply_hotfix( bool ptr );
  };

  bool register_hotfix( const std::string&, const std::string&, unsigned = HOTFIX_FLAG_DEFAULT );
  spell_hotfix_entry_t& register_spell( const std::string&, const std::string&, unsigned, unsigned = hotfix::HOTFIX_FLAG_DEFAULT );
  effect_hotfix_entry_t& register_effect( const std::string&, const std::string&, unsigned, unsigned = hotfix::HOTFIX_FLAG_DEFAULT );

  void apply();
  std::string to_str();

  void add_hotfix_spell( spell_data_t* spell, bool ptr = false );
  const spell_data_t* find_spell( const spell_data_t* dbc_spell, bool ptr = false );
  const spelleffect_data_t* find_effect( const spelleffect_data_t* dbc_effect, bool ptr = false );
}

namespace dbc_override
{
  bool register_effect( sim_t*, unsigned, const std::string&, double );
  bool register_spell( sim_t*, unsigned, const std::string&, double );

  const spell_data_t* find_spell( unsigned, bool ptr = false );
}

// ==========================================================================
// Spell Power Data - SpellPower.dbc
// ==========================================================================

struct spellpower_data_t
{
public:
  unsigned _id;
  unsigned _spell_id;
  unsigned _aura_id; // Spell id for the aura during which this power type is active
  int      _power_e;
  int      _cost;
  double   _cost_2;
  int      _cost_per_second; // Unsure
  double   _cost_per_second_2;

  resource_e resource() const
  { return util::translate_power_type( type() ); }

  unsigned id() const
  { return _id; }

  unsigned spell_id() const
  { return _spell_id; }

  unsigned aura_id() const
  { return _aura_id; }

  power_e type() const
  { return static_cast< power_e >( _power_e ); }

  double cost_divisor( bool percentage ) const
  {
    switch ( type() )
    {
      case POWER_MANA:
      case POWER_SOUL_SHARDS:
        return 100.0;
      case POWER_RAGE:
      case POWER_RUNIC_POWER:
      case POWER_BURNING_EMBER:
        return 10.0;
      case POWER_DEMONIC_FURY:
        return percentage ? 0.1 : 1.0;  // X% of 1000 ("base" demonic fury) is X divided by 0.1
      default:
        return 1.0;
    }
  }

  double cost() const
  {
    double cost = _cost > 0 ? _cost : _cost_2;

    return cost / cost_divisor( ! ( _cost > 0 ) );
  }

  double cost_per_second() const
  {
    double cost = _cost_per_second > 0 ? _cost_per_second : _cost_per_second_2;

    return cost / cost_divisor( ! ( _cost_per_second > 0 ) );
  }

  static spellpower_data_t* nil();
  static spellpower_data_t* list( bool ptr = false );
  static void               link( bool ptr = false );
};

class spellpower_data_nil_t : public spellpower_data_t
{
public:
  spellpower_data_nil_t() :
    spellpower_data_t()
  {}
  static spellpower_data_nil_t singleton;
};

inline spellpower_data_t* spellpower_data_t::nil()
{ return &spellpower_data_nil_t::singleton; }

// ==========================================================================
// Spell Effect Data - SpellEffect.dbc
// ==========================================================================

struct spelleffect_data_t
{
public:
  unsigned         _id;              // Effect id
  unsigned         _flags;           // Unused for now, 0x00 for all
  unsigned         _spell_id;        // Spell this effect belongs to
  unsigned         _index;           // Effect index for the spell
  effect_type_t    _type;            // Effect type
  effect_subtype_t _subtype;         // Effect sub-type
  // SpellScaling.dbc
  double           _m_avg;           // Effect average spell scaling multiplier
  double           _m_delta;         // Effect delta spell scaling multiplier
  double           _m_unk;           // Unused effect scaling multiplier
  //
  double           _sp_coeff;           // Effect coefficient
  double           _ap_coeff;        // Effect attack power coefficient
  double           _amplitude;       // Effect amplitude (e.g., tick time)
  // SpellRadius.dbc
  double           _radius;          // Minimum spell radius
  double           _radius_max;      // Maximum spell radius
  //
  int              _base_value;      // Effect value
  int              _misc_value;      // Effect miscellaneous value
  int              _misc_value_2;    // Effect miscellaneous value 2
  unsigned         _class_flags[NUM_CLASS_FAMILY_FLAGS]; // Class family flags
  unsigned         _trigger_spell_id;// Effect triggers this spell id
  double           _m_chain;         // Effect chain multiplier
  double           _pp_combo_points; // Effect points per combo points
  double           _real_ppl;        // Effect real points per level
  int              _die_sides;       // Effect damage range

  // Pointers for runtime linking
  spell_data_t* _spell;
  spell_data_t* _trigger_spell;

  bool ok() const
  { return _id != 0; }

  unsigned id() const
  { return _id; }

  unsigned index() const
  { return _index; }

  unsigned spell_id() const
  { return _spell_id; }

  unsigned spell_effect_num() const
  { return _index; }

  effect_type_t type() const
  { return _type; }

  effect_subtype_t subtype() const
  { return _subtype; }

  int base_value() const
  { return _base_value; }

  double percent() const
  { return _base_value * ( 1 / 100.0 ); }

  timespan_t time_value() const
  { return timespan_t::from_millis( _base_value ); }

  resource_e resource_gain_type() const
  { return util::translate_power_type( static_cast< power_e >( misc_value1() ) ); }

  double resource( resource_e resource_type ) const
  {
    switch ( resource_type )
    {
      case RESOURCE_RUNIC_POWER:
      case RESOURCE_RAGE:
        return base_value() * ( 1 / 10.0 );
      case RESOURCE_MANA:
        return base_value() * ( 1 / 100.0 );
      default:
        return base_value();
    }
  }

  double mastery_value() const
  { return _sp_coeff * ( 1 / 100.0 ); }

  int misc_value1() const
  { return _misc_value; }

  int misc_value2() const
  { return _misc_value_2; }

  unsigned trigger_spell_id() const
  { return _trigger_spell_id; }

  double chain_multiplier() const
  { return _m_chain; }

  double m_average() const
  { return _m_avg; }

  double m_delta() const
  { return _m_delta; }

  double m_unk() const
  { return _m_unk; }

  double sp_coeff() const
  { return _sp_coeff; }

  double ap_coeff() const
  { return _ap_coeff; }

  timespan_t period() const
  { return timespan_t::from_millis( _amplitude ); }

  double radius() const
  { return _radius; }

  double radius_max() const
  { return std::max( _radius_max, radius() ); }

  double pp_combo_points() const
  { return _pp_combo_points; }

  double real_ppl() const
  { return _real_ppl; }

  int die_sides() const
  { return _die_sides; }

  bool class_flag( unsigned flag ) const
  {
    unsigned index = flag / 32;
    unsigned bit = flag % 32;

    assert( index < sizeof_array( _class_flags ) );
    return ( _class_flags[ index ] & ( 1u << bit ) ) != 0;
  }

  unsigned class_flags( unsigned idx ) const
  {
    assert( idx < NUM_CLASS_FAMILY_FLAGS );

    return _class_flags[ idx ];
  }

  double average( const player_t* p, unsigned level = 0 ) const;
  double delta( const player_t* p, unsigned level = 0 ) const;
  double bonus( const player_t* p, unsigned level = 0 ) const;
  double min( const player_t* p, unsigned level = 0 ) const;
  double max( const player_t* p, unsigned level = 0 ) const;

  double average( const item_t* item ) const;
  double delta( const item_t* item ) const;
  double min( const item_t* item ) const;
  double max( const item_t* item ) const;

  double average( const item_t& item ) const { return average( &item ); }
  double delta( const item_t& item ) const { return delta( &item ); }
  double min( const item_t& item ) const { return min( &item ); }
  double max( const item_t& item ) const { return max( &item ); }

  bool override_field( const std::string& field, double value );
  double get_field( const std::string& field ) const;

  spell_data_t* spell() const;

  spell_data_t* trigger() const;

  static spelleffect_data_t* nil();
  static spelleffect_data_t* find( unsigned, bool ptr = false );
  static spelleffect_data_t* list( bool ptr = false );
  static void                link( bool ptr = false );
private:
  double scaled_average( double budget, unsigned level ) const;
  double scaled_delta( double budget ) const;
  double scaled_min( double avg, double delta ) const;
  double scaled_max( double avg, double delta ) const;
};

// ==========================================================================
// Spell Data
// ==========================================================================

#ifdef __OpenBSD__
#pragma pack(1)
#else
#pragma pack( push, 1 )
#endif
struct spell_data_t
{
private:
  friend void dbc::init();
  friend void dbc::de_init();
  static void link( bool ptr );
public:
  const char* _name;               // Spell name from Spell.dbc stringblock (enGB)
  unsigned    _id;                 // Spell ID in dbc
  unsigned    _flags;              // Unused for now, 0x00 for all
  double      _prj_speed;          // Projectile Speed
  unsigned    _school;             // Spell school mask
  unsigned    _class_mask;         // Class mask for spell
  unsigned    _race_mask;          // Racial mask for the spell
  int         _scaling_type;       // Array index for gtSpellScaling.dbc. -1 means the first non-class-specific sub array, and so on, 0 disabled
  unsigned    _max_scaling_level;  // Max scaling level(?), 0 == no restrictions, otherwise min( player_level, max_scaling_level )
  // SpellLevels.dbc
  unsigned    _spell_level;        // Spell learned on level. NOTE: Only accurate for "class abilities"
  unsigned    _max_level;          // Maximum level for scaling
  // SpellRange.dbc
  double      _min_range;          // Minimum range in yards
  double      _max_range;          // Maximum range in yards
  // SpellCooldown.dbc
  unsigned    _cooldown;           // Cooldown in milliseconds
  unsigned    _gcd;                // GCD in milliseconds
  // SpellCategory.dbc
  unsigned    _charges;            // Number of charges
  unsigned    _charge_cooldown;    // Cooldown duration of charges
  // SpellCategories.dbc
  unsigned    _category;           // Spell category (for shared cooldowns, effects?)
  // SpellDuration.dbc
  double      _duration;           // Spell duration in milliseconds
  // SpellRuneCost.dbc
  unsigned    _rune_cost;          // Bitmask of rune cost 0x1, 0x2 = Blood | 0x4, 0x8 = Unholy | 0x10, 0x20 = Frost
  unsigned    _runic_power_gain;   // Amount of runic power gained ( / 10 )
  // SpellAuraOptions.dbc
  unsigned    _max_stack;          // Maximum stack size for spell
  unsigned    _proc_chance;        // Spell proc chance in percent
  unsigned    _proc_charges;       // Per proc charge amount
  unsigned    _proc_flags;         // Proc flags
  unsigned    _internal_cooldown;  // ICD
  double      _rppm;               // Base real procs per minute
  // SpellEquippedItems.dbc
  unsigned    _equipped_class;
  unsigned    _equipped_invtype_mask;
  unsigned    _equipped_subclass_mask;
  // SpellScaling.dbc
  int         _cast_min;           // Minimum casting time in milliseconds
  int         _cast_max;           // Maximum casting time in milliseconds
  int         _cast_div;           // A divisor used in the formula for casting time scaling (20 always?)
  double      _c_scaling;          // A scaling multiplier for level based scaling
  unsigned    _c_scaling_level;    // A scaling divisor for level based scaling
  // SpecializationSpells.dbc
  unsigned    _replace_spell_id;
  // Spell.dbc flags
  unsigned    _attributes[NUM_SPELL_FLAGS];// Spell.dbc "flags", record field 1..10, note that 12694 added a field here after flags_7
  unsigned    _class_flags[NUM_CLASS_FAMILY_FLAGS]; // SpellClassOptions.dbc flags
  unsigned    _class_flags_family; // SpellClassOptions.dbc spell family
  const char* _desc;               // Spell.dbc description stringblock
  const char* _tooltip;            // Spell.dbc tooltip stringblock
  // SpellDescriptionVariables.dbc
  const char* _desc_vars;          // Spell description variable stringblock, if present
  // SpellIcon.dbc
  const char* _icon;
  const char* _active_icon;
  const char* _rank_str;

  // Pointers for runtime linking
  std::vector<const spelleffect_data_t*>* _effects;
  std::vector<const spellpower_data_t*>*  _power;

  spell_data_t*    _driver; // The triggered spell's driver

  // Direct member access functions
  uint32_t category() const
  { return _category; }

  uint32_t class_mask() const
  { return _class_mask; }

  timespan_t cooldown() const
  { return timespan_t::from_millis( _cooldown ); }

  unsigned charges() const
  { return _charges; }

  timespan_t charge_cooldown() const
  { return timespan_t::from_millis( _charge_cooldown ); }

  const char* desc() const
  { return ok() ? _desc : ""; }

  const char* desc_vars() const
  { return ok() ? _desc_vars : ""; }

  timespan_t duration() const
  { return timespan_t::from_millis( _duration ); }

  double extra_coeff() const
  { return 0; }

  timespan_t gcd() const
  { return timespan_t::from_millis( _gcd ); }

  unsigned id() const
  { return _id; }

  uint32_t initial_stacks() const
  { return _proc_charges; }

  uint32_t race_mask() const
  { return _race_mask; }

  uint32_t level() const
  { return _spell_level; }

  const char* name_cstr() const
  { return ok() ? _name : ""; }

  uint32_t max_level() const
  { return _max_level; }

  uint32_t max_stacks() const
  { return _max_stack; }

  double missile_speed() const
  { return _prj_speed; }

  double min_range() const
  { return _min_range; }

  double max_range() const
  { return _max_range; }

  double proc_chance() const
  { return _proc_chance * ( 1 / 100.0 ); }

  unsigned proc_flags() const
  { return _proc_flags; }

  timespan_t internal_cooldown() const
  { return timespan_t::from_millis( _internal_cooldown ); }

  double real_ppm() const
  { return _rppm; }

  const char* rank_str() const
  { return ok() ? _rank_str : ""; }

  unsigned replace_spell_id() const
  { return _replace_spell_id; }

  uint32_t rune_cost() const
  { return _rune_cost; }

  double runic_power_gain() const
  { return _runic_power_gain * ( 1 / 10.0 ); }

  double scaling_multiplier() const
  { return _c_scaling; }

  unsigned scaling_threshold() const
  { return _c_scaling_level; }

  uint32_t school_mask() const
  { return _school; }

  const char* tooltip() const
  { return ok() ? _tooltip : ""; }

  // Helper functions
  size_t effect_count() const
  { assert( _effects ); return _effects -> size(); }

  size_t power_count() const
  { return _power ? _power -> size() : 0; }

  bool found() const
  { return ( this != not_found() ); }

  school_e  get_school_type() const
  { return dbc::get_school_type( as<uint32_t>( _school ) ); }

  bool is_level( uint32_t level ) const
  { return level >= _spell_level; }

  bool in_range( double range ) const
  { return range >= _min_range && range <= _max_range; }

  bool is_race( race_e r ) const
  {
    unsigned mask = util::race_mask( r );
    return ( _race_mask & mask ) == mask;
  }

  bool ok() const
  { return _id != 0; }

  // Composite functions
  const spelleffect_data_t& effectN( size_t idx ) const
  {
    assert( _effects );
    assert( idx > 0 && "effect index must not be zero or less" );

    if ( this == spell_data_t::nil() || this == spell_data_t::not_found() )
      return *spelleffect_data_t::nil();

    assert( idx <= _effects -> size() && "effect index out of bound!" );

    return *( ( *_effects )[ idx - 1 ] );
  }

  const spellpower_data_t& powerN( size_t idx ) const
  {
    if ( _power )
    {
      assert( idx > 0 && _power && idx <= _power -> size() );

      return *_power -> at( idx - 1 );
    }

    return *spellpower_data_t::nil();
  }

  const spellpower_data_t& powerN( power_e pt ) const
  {
    assert( pt >= POWER_HEALTH && pt < POWER_MAX );
    if ( _power )
    {
      for ( size_t i = 0; i < _power -> size(); i++ )
      {
        if ( _power -> at( i ) -> _power_e == pt )
          return *_power -> at( i );
      }
    }

    return *spellpower_data_t::nil();
  }

  bool is_class( player_e c ) const
  {
    if ( ! _class_mask )
      return true;

    unsigned mask = util::class_id_mask( c );
    return ( _class_mask & mask ) == mask;
  }

  player_e scaling_class() const
  {
    switch ( _scaling_type )
    {
      case -4: return PLAYER_SPECIAL_SCALE4;
      case -3: return PLAYER_SPECIAL_SCALE3;
      case -2: return PLAYER_SPECIAL_SCALE2;
      case -1: return PLAYER_SPECIAL_SCALE;
      case 1:  return WARRIOR;
      case 2:  return PALADIN;
      case 3:  return HUNTER;
      case 4:  return ROGUE;
      case 5:  return PRIEST;
      case 6:  return DEATH_KNIGHT;
      case 7:  return SHAMAN;
      case 8:  return MAGE;
      case 9:  return WARLOCK;
      case 10: return MONK;
      case 11: return DRUID;
      default: break;
    }

    return PLAYER_NONE;
  }

  unsigned max_scaling_level() const
  { return _max_scaling_level; }

  timespan_t cast_time( uint32_t level ) const
  {
    if ( _cast_div < 0 )
    {
      return timespan_t::from_millis( std::max( 0, _cast_min ) );
    }

    if ( level >= as<uint32_t>( _cast_div ) )
      return timespan_t::from_millis( _cast_max );

    return timespan_t::from_millis( _cast_min + ( _cast_max - _cast_min ) * ( level - 1 ) / ( double )( _cast_div - 1 ) );
  }

  double cost( power_e pt ) const
  {
    if ( _power )
    {
      for ( size_t i = 0; i < _power -> size(); i++ )
      {
        if ( ( *_power )[ i ] -> _power_e == pt )
          return ( *_power )[ i ] -> cost();
      }
    }

    return 0.0;
  }

  uint32_t effect_id( uint32_t effect_num ) const
  {
    assert( _effects );
    assert( effect_num >= 1 && effect_num <= _effects -> size() );
    return ( *_effects )[ effect_num - 1 ] -> id();
  }

  bool flags( spell_attribute_e f ) const
  {
    unsigned bit = static_cast<unsigned>( f ) & 0x1Fu;
    unsigned index = ( static_cast<unsigned>( f ) >> 8 ) & 0xFFu;
    uint32_t mask = 1u << bit;

    assert( index < sizeof_array( _attributes ) );

    return ( _attributes[ index ] & mask ) != 0;
  }

  bool class_flag( unsigned flag ) const
  {
    unsigned index = flag / 32;
    unsigned bit = flag % 32;

    assert( index < sizeof_array( _class_flags ) );
    return ( _class_flags[ index ] & ( 1u << bit ) ) != 0;
  }

  unsigned attribute( unsigned idx ) const
  { assert( idx < sizeof_array( _attributes ) ); return _attributes[ idx ]; }

  unsigned class_flags( unsigned idx ) const
  { assert( idx < sizeof_array( _class_flags ) ); return _class_flags[ idx ]; }

  unsigned class_family() const
  { return _class_flags_family; }

  bool override_field( const std::string& field, double value );
  double get_field( const std::string& field ) const;

  bool valid_item_enchantment( inventory_type inv_type ) const
  {
    if ( ! _equipped_invtype_mask )
      return true;

    unsigned invtype_mask = 1 << inv_type;
    if ( _equipped_invtype_mask & invtype_mask )
      return true;

    return false;
  }

  std::string to_str() const
  {
    std::ostringstream s;

    s << " (ok=" << ( ok() ? "true" : "false" ) << ")";
    s << " id=" << id();
    s << " name=" << name_cstr();
    s << " school=" << util::school_type_string( get_school_type() );
    return s.str();
  }

  bool affected_by( const spell_data_t* ) const;
  bool affected_by( const spelleffect_data_t* ) const;
  bool affected_by( const spelleffect_data_t& ) const;

  spell_data_t* driver() const
  { return _driver ? _driver : spell_data_t::nil(); }

  // static functions
  static spell_data_t* nil();
  static spell_data_t* not_found();
  static spell_data_t* find( const char* name, bool ptr = false );
  static spell_data_t* find( unsigned id, bool ptr = false );
  static spell_data_t* find( unsigned id, const char* confirmation, bool ptr = false );
  static spell_data_t* list( bool ptr = false );
  static void de_link( bool ptr = false );
} SC_PACKED_STRUCT;
#ifdef __OpenBSD__
#pragma pack()
#else
#pragma pack( pop )
#endif

// ==========================================================================
// Spell Effect Data Nil
// ==========================================================================

class spelleffect_data_nil_t : public spelleffect_data_t
{
public:
  spelleffect_data_nil_t() : spelleffect_data_t()
  { _spell = _trigger_spell = spell_data_t::not_found(); }

  static spelleffect_data_nil_t singleton;
};

inline spelleffect_data_t* spelleffect_data_t::nil()
{ return &spelleffect_data_nil_t::singleton; }

// ==========================================================================
// Spell Data Nil / Not found
// ==========================================================================

/* Empty spell_data container
 */
class spell_data_nil_t : public spell_data_t
{
public:
  spell_data_nil_t() : spell_data_t()
  {
    _effects = new std::vector< const spelleffect_data_t* >();
  }

  ~spell_data_nil_t()
  { delete _effects; }

  static spell_data_nil_t singleton;
};

inline spell_data_t* spell_data_t::nil()
{ return &spell_data_nil_t::singleton; }

/* Empty spell data container, which is used to return a "not found" state
 */
class spell_data_not_found_t : public spell_data_t
{
public:
  spell_data_not_found_t() : spell_data_t()
  {
    _effects = new std::vector< const spelleffect_data_t* >();
  }

  ~spell_data_not_found_t()
  { delete _effects; }

  static spell_data_not_found_t singleton;
};

inline spell_data_t* spell_data_t::not_found()
{ return &spell_data_not_found_t::singleton; }

inline spell_data_t* spelleffect_data_t::spell() const
{
  return _spell ? _spell : spell_data_t::not_found();
}

inline spell_data_t* spelleffect_data_t::trigger() const
{
  return _trigger_spell ? _trigger_spell : spell_data_t::not_found();
}

// ==========================================================================
// Talent Data
// ==========================================================================

struct talent_data_t
{
public:
  const char * _name;        // Talent name
  unsigned     _id;          // Talent id
  unsigned     _flags;       // Unused for now, 0x00 for all
  unsigned     _m_class;     // Class mask
  unsigned     _spec;        // Specialization
  unsigned     _col;         // Talent column
  unsigned     _row;         // Talent row
  unsigned     _spell_id;    // Talent spell
  unsigned     _replace_id;  // Talent replaces the following spell id

  // Pointers for runtime linking
  const spell_data_t* spell1;

  // Direct member access functions
  unsigned id() const
  { return _id; }

  const char* name_cstr() const
  { return _name; }

  unsigned col() const
  { return _col; }

  unsigned row() const
  { return _row; }

  unsigned spell_id() const
  { return _spell_id; }

  unsigned replace_id() const
  { return _replace_id; }

  unsigned mask_class() const
  { return _m_class; }

  unsigned spec() const
  { return _spec; }

  specialization_e specialization() const
  { return static_cast<specialization_e>( _spec ); }

  // composite access functions

  const spell_data_t* spell() const
  { return spell1 ? spell1 : spell_data_t::nil(); }

  bool is_class( player_e c ) const
  {
    unsigned mask = util::class_id_mask( c );

    if ( mask == 0 )
      return false;

    return ( ( _m_class & mask ) == mask );
  }

  // static functions
  static talent_data_t* nil();
  static talent_data_t* find( unsigned, bool ptr = false );
  static talent_data_t* find( unsigned, const char* confirmation, bool ptr = false );
  static talent_data_t* find( const char* name, specialization_e spec, bool ptr = false );
  static talent_data_t* find_tokenized( const char* name, specialization_e spec, bool ptr = false );
  static talent_data_t* find( player_e c, unsigned int row, unsigned int col, specialization_e spec, bool ptr = false );
  static talent_data_t* list( bool ptr = false );
  static void           link( bool ptr = false );
};

class talent_data_nil_t : public talent_data_t
{
public:
  talent_data_nil_t() :
    talent_data_t()
  { spell1 = spell_data_t::nil(); }

  static talent_data_nil_t singleton;
};

inline talent_data_t* talent_data_t::nil()
{ return &talent_data_nil_t::singleton; }


// If we don't have any ptr data, don't bother to check if ptr is true
#if SC_USE_PTR
inline bool maybe_ptr( bool ptr ) { return ptr; }
#else
inline bool maybe_ptr( bool ) { return false; }
#endif

// ==========================================================================
// General Database with state
// ==========================================================================

/* Database access with a state ( bool ptr )
 */
class dbc_t
{
public:
  bool ptr;

private:
  typedef std::unordered_map<uint32_t, uint32_t> id_map_t;
  id_map_t replaced_ids;
public:
  uint32_t replaced_id( uint32_t id_spell ) const;
  bool replace_id( uint32_t id_spell, uint32_t replaced_by_id );

  dbc_t( bool ptr = false ) :
    ptr( ptr ) { }

  int build_level() const
  { return dbc::build_level( ptr ); }

  const char* wow_version() const
  { return dbc::wow_version( ptr ); }

  const char* wow_ptr_status() const
  { return dbc::wow_ptr_status( ptr ); }

  const item_data_t* items() const
  { return dbc::items( ptr ); }

  std::size_t n_items() const
  { return dbc::n_items( ptr ); }

  const item_enchantment_data_t* item_enchantments() const
  { return dbc::item_enchantments( ptr ); }

  std::size_t n_item_enchantments() const
  { return dbc::n_item_enchantments( ptr ); }

  const gem_property_data_t* gem_properties() const
  { return dbc::gem_properties( ptr ); }

  bool add_token( unsigned int id_spell, const std::string& token_name ) const
  { return dbc::add_token( id_spell, token_name, ptr ); }

  // Game data table access
  double melee_crit_base( player_e t, unsigned level ) const;
  double melee_crit_base( pet_e t, unsigned level ) const;
  double spell_crit_base( player_e t, unsigned level ) const;
  double spell_crit_base( pet_e t, unsigned level ) const;
  double dodge_base( player_e t ) const;
  double dodge_base( pet_e t ) const;
  double regen_base( player_e t, unsigned level ) const;
  double regen_base( pet_e t, unsigned level ) const;
  double resource_base( player_e t, unsigned level ) const;
  double health_base( player_e t, unsigned level ) const;
  stat_data_t& attribute_base( player_e t, unsigned level ) const;
  stat_data_t& attribute_base( pet_e t, unsigned level ) const;
  stat_data_t& race_base( race_e r ) const;
  stat_data_t& race_base( pet_e t ) const;
  double parry_factor( player_e t ) const;
  double dodge_factor( player_e t ) const;
  double miss_factor( player_e t ) const;
  double block_factor( player_e t ) const;
  double vertical_stretch( player_e t ) const;
  double horizontal_shift( player_e t ) const;

  double spell_scaling( player_e t, unsigned level ) const;
  double melee_crit_scaling( player_e t, unsigned level ) const;
  double melee_crit_scaling( pet_e t, unsigned level ) const;
  double spell_crit_scaling( player_e t, unsigned level ) const;
  double spell_crit_scaling( pet_e t, unsigned level ) const;
  double regen_spirit( player_e t, unsigned level ) const;
  double regen_spirit( pet_e t, unsigned level ) const;
  double health_per_stamina( unsigned level ) const;
  double item_socket_cost( unsigned ilevel ) const;
  double real_ppm_coefficient( specialization_e, unsigned ) const;
  double armor_mitigation_constant( unsigned level ) const;

  double combat_rating( unsigned combat_rating_id, unsigned level ) const;
  double oct_combat_rating( unsigned combat_rating_id, player_e t ) const;

  int resolve_item_scaling( unsigned level ) const;
  item_bonus_tree_entry_t& resolve_item_bonus_tree_data( unsigned level ) const;
  item_bonus_node_entry_t& resolve_item_bonus_map_data( unsigned level ) const;
  double resolve_level_scaling( unsigned level ) const;
  double avoid_per_str_agi_by_level( unsigned level ) const;

private:
  template <typename T>
  const T* find_by_id( unsigned id ) const
  {
    const T* item = T::find( id, ptr );
    assert( item && ( item -> id() == id || item -> id() == 0 ) );
    return item;
  }

public:
  // Always returns non-NULL.
  const spell_data_t*            spell( unsigned spell_id ) const
  { return find_by_id<spell_data_t>( spell_id ); }

  // Always returns non-NULL.
  const spelleffect_data_t*      effect( unsigned effect_id ) const
  { return find_by_id<spelleffect_data_t>( effect_id ); }

  // Always returns non-NULL.
  const talent_data_t*           talent( unsigned talent_id ) const
  { return find_by_id<talent_data_t>( talent_id ); }

  const item_data_t*             item( unsigned item_id ) const;
  const random_suffix_data_t&    random_suffix( unsigned suffix_id ) const;
  const item_enchantment_data_t& item_enchantment( unsigned enchant_id ) const;
  const gem_property_data_t&     gem_property( unsigned gem_id ) const;

  const random_prop_data_t&      random_property( unsigned ilevel ) const;
  unsigned                            random_property_max_level() const;
  const item_scale_data_t&       item_damage_1h( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_2h( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_caster_1h( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_caster_2h( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_ranged( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_thrown( unsigned ilevel ) const;
  const item_scale_data_t&       item_damage_wand( unsigned ilevel ) const;

  const item_scale_data_t&       item_armor_quality( unsigned ilevel ) const;
  const item_scale_data_t&       item_armor_shield( unsigned ilevel ) const;
  const item_armor_type_data_t&  item_armor_total( unsigned ilevel ) const;
  const item_armor_type_data_t&  item_armor_inv_type( unsigned inv_type ) const;

  const item_upgrade_t&          item_upgrade( unsigned upgrade_id ) const;
  const item_upgrade_rule_t&     item_upgrade_rule( unsigned item_id, unsigned upgrade_level ) const;
  const rppm_modifier_t&         real_ppm_modifier( specialization_e spec, unsigned spell_id ) const;

  std::vector<const item_bonus_entry_t*> item_bonus( unsigned bonus_id ) const;

  // Derived data access
  unsigned num_tiers() const;

  unsigned class_ability( unsigned class_id, unsigned tree_id, unsigned n ) const;
  unsigned pet_ability( unsigned class_id, unsigned n ) const;
  unsigned class_ability_tree_size() const;
  unsigned class_ability_size() const;
  unsigned class_max_size() const;

  unsigned race_ability( unsigned race_id, unsigned class_id, unsigned n ) const;
  unsigned race_ability_size() const;
  unsigned race_ability_tree_size() const;

  unsigned specialization_ability( unsigned class_id, unsigned tree_id, unsigned n ) const;
  unsigned specialization_ability_size() const;
  unsigned specialization_max_per_class() const;
  unsigned specialization_max_class() const;
  bool     ability_specialization( uint32_t spell_id, std::vector<specialization_e>& spec_list ) const;

  unsigned perk_ability_size() const;
  unsigned perk_ability( unsigned class_id, unsigned tree_id, unsigned n ) const;

  unsigned mastery_ability( unsigned class_id, unsigned tree_id, unsigned n ) const;
  unsigned mastery_ability_size() const;
  int      mastery_ability_tree( player_e c, uint32_t spell_id ) const;

  unsigned glyph_spell( unsigned class_id, unsigned glyph_e, unsigned n ) const;
  unsigned glyph_spell_size() const;

  unsigned set_bonus_spell( unsigned class_id, unsigned tier, unsigned n ) const;
  unsigned set_bonus_spell_size() const;

  // Helper methods
  double   weapon_dps( unsigned item_id, unsigned ilevel = 0 ) const;
  double   weapon_dps( const item_data_t*, unsigned ilevel = 0 ) const;

  double   effect_average( unsigned effect_id, unsigned level ) const;
  double   effect_average( const spelleffect_data_t* effect, unsigned level ) const;
  double   effect_delta( unsigned effect_id, unsigned level ) const;
  double   effect_delta( const spelleffect_data_t* effect, unsigned level ) const;

  double   effect_min( unsigned effect_id, unsigned level ) const;
  double   effect_min( const spelleffect_data_t* effect, unsigned level ) const;
  double   effect_max( unsigned effect_id, unsigned level ) const;
  double   effect_max( const spelleffect_data_t* effect, unsigned level ) const;
  double   effect_bonus( unsigned effect_id, unsigned level ) const;
  double   effect_bonus( const spelleffect_data_t* effect, unsigned level ) const;

  unsigned talent_ability_id( player_e c, specialization_e spec_id, const char* spell_name, bool name_tokenized = false ) const;
  unsigned class_ability_id( player_e c, specialization_e spec_id, const char* spell_name ) const;
  unsigned pet_ability_id( player_e c, const char* spell_name ) const;
  unsigned race_ability_id( player_e c, race_e r, const char* spell_name ) const;
  unsigned specialization_ability_id( specialization_e spec_id, const char* spell_name ) const;
  unsigned perk_ability_id( specialization_e spec_id, const char* spell_name ) const;
  unsigned perk_ability_id( specialization_e spec_id, size_t perk_index ) const;
  unsigned mastery_ability_id( specialization_e spec, const char* spell_name ) const;
  unsigned mastery_ability_id( specialization_e spec, uint32_t idx ) const;
  specialization_e mastery_specialization( const player_e c, uint32_t spell_id ) const;

  unsigned glyph_spell_id( player_e c, const char* spell_name ) const;
  unsigned glyph_spell_id( unsigned property_id ) const;
  unsigned set_bonus_spell_id( player_e c, const char* spell_name, int tier = -1 ) const;

  specialization_e class_ability_specialization( const player_e c, uint32_t spell_id ) const;

  bool     is_class_ability( uint32_t spell_id ) const;
  bool     is_race_ability( uint32_t spell_id ) const;
  bool     is_specialization_ability( uint32_t spell_id ) const;
  bool     is_mastery_ability( uint32_t spell_id ) const;
  bool     is_glyph_spell( uint32_t spell_id ) const;
  bool     is_set_bonus_spell( uint32_t spell_id ) const;

  specialization_e spec_by_spell( uint32_t spell_id ) const;

  bool spec_idx( specialization_e spec_id, uint32_t& class_idx, uint32_t& spec_index ) const;
  specialization_e spec_by_idx( const player_e c, unsigned idx ) const;
  double rppm_coefficient( specialization_e spec, unsigned spell_id ) const;

  unsigned item_upgrade_ilevel( unsigned item_id, unsigned upgrade_level ) const;

  std::vector< const spell_data_t* > effect_affects_spells( unsigned, const spelleffect_data_t* ) const;
  std::vector< const spelleffect_data_t* > effects_affecting_spell( const spell_data_t* ) const;

  // Heirloomage and misc scaling hijinxery
  const scaling_stat_distribution_t* scaling_stat_distribution( unsigned id );
  std::pair<const curve_point_t*, const curve_point_t*> curve_point( unsigned curve_id, double value );

};
// ==========================================================================
// Indices to provide log time, constant space access to spells, effects, and talents by id.
// ==========================================================================

/* id_function_policy and id_member_policy are here to give a standard interface
 * of accessing the id of a data type.
 * Eg. spell_data_t on which the id_function_policy is used has a function 'id()' which returns its id
 * and item_data_t on which id_member_policy is used has a member 'id' which stores its id.
 */
struct id_function_policy
{
  template <typename T> static unsigned id( const T& t )
  { return static_cast<unsigned>( t.id() ); }
};

struct id_member_policy
{
  template <typename T> static unsigned id( const T& t )
  { return static_cast<unsigned>( t.id ); }
};

template<typename T, typename KeyPolicy = id_function_policy>
struct id_compare
{
  bool operator () ( const T& t, unsigned int id ) const
  { return KeyPolicy::id( t ) < id; }
  bool operator () ( unsigned int id, const T& t ) const
  { return id < KeyPolicy::id( t ); }
  bool operator () ( const T& l, const T& r ) const
  { return KeyPolicy::id( l ) < KeyPolicy::id( r ); }
};

template <typename T, typename KeyPolicy = id_function_policy>
class dbc_index_t
{
private:
  typedef std::pair<T*, T*> index_t; // first = lowest data; second = highest data
// array of size 1 or 2, depending on whether we have PTR data
#if SC_USE_PTR == 0
  index_t idx[ 1 ];
#else
  index_t idx[ 2 ];
#endif

  /* populate idx with pointer to lowest and highest data from a given list
   */
  void populate( index_t& idx, T* list )
  {
    assert( list );
    idx.first = list;
    for ( unsigned last_id = 0; KeyPolicy::id( *list ); last_id = KeyPolicy::id( *list ), ++list )
    {
      // Validate the input range is in fact sorted by id.
      assert( KeyPolicy::id( *list ) > last_id ); ( void )last_id;
    }
    idx.second = list;
  }

public:
  // Initialize index from given list
  void init( T* list, bool ptr )
  {
    assert( ! initialized( maybe_ptr( ptr ) ) );
    populate( idx[ maybe_ptr( ptr ) ], list );
  }

  // Initialize index under the assumption that 'T::list( bool ptr )' returns a list of data
  void init()
  {
    init( T::list( false ), false );
    if ( SC_USE_PTR )
      init( T::list( true ), true );
  }

  bool initialized( bool ptr = false ) const
  { return idx[ maybe_ptr( ptr ) ].first != 0; }

  // Return the item with the given id, or NULL.
  // Always returns non-NULL.
  T* get( bool ptr, unsigned id ) const
  {
    assert( initialized( maybe_ptr( ptr ) ) );
    T* p = std::lower_bound( idx[ maybe_ptr( ptr ) ].first, idx[ maybe_ptr( ptr ) ].second, id, id_compare<T, KeyPolicy>() );
    if ( p != idx[ maybe_ptr( ptr ) ].second && KeyPolicy::id( *p ) == id )
      return p;
    else
      return NULL;
  }
};

template <typename T, typename Filter, typename KeyPolicy = id_function_policy>
class filtered_dbc_index_t
{
#if SC_USE_PTR == 0
  std::vector<T*> __filtered_index[ 1 ];
#else
  std::vector<T*> __filtered_index[ 2 ];
#endif
  Filter f;

public:
  typedef typename std::vector<T*>::const_iterator citerator;

  citerator begin( bool ptr ) const
  { return __filtered_index[ maybe_ptr( ptr ) ].begin(); }

  citerator end( bool ptr ) const
  { return __filtered_index[ maybe_ptr( ptr ) ].end(); }

  // Initialize index from given list
  void init( T* list, bool ptr )
  {
    T* i = list;

    while ( KeyPolicy::id( *i ) )
    {
      if ( f( i ) )
      {
        __filtered_index[ maybe_ptr( ptr ) ].push_back( i );
      }

      i++;
    }
  }

  const T* get( bool ptr, const std::function<bool(const T*)> f ) const
  {
    for ( citerator i = begin( ptr ), e = end( ptr ); i < e; ++i )
    {
      if ( f( *i ) )
      {
        return *i;
      }
    }

    return 0;
  }

  const T* get( bool ptr, unsigned id ) const
  {
    const T* p = std::lower_bound( __filtered_index[ maybe_ptr( ptr ) ].begin(),
                                   __filtered_index[ maybe_ptr( ptr ) ].end(),
                                   id, id_compare<T, KeyPolicy>() );

    if ( p != __filtered_index[ maybe_ptr( ptr ) ].end() && KeyPolicy::id( *p ) == id )
      return p;
    else
      return NULL;
  }
};

#endif // SC_DBC_HPP
