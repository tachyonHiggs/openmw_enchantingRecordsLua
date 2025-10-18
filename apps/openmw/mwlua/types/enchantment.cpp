#include "types.hpp"

#include "modelproperty.hpp"

#include <components/esm3/loadench.hpp>
#include <components/lua/luastate.hpp>
#include <components/lua/util.hpp>
#include <components/misc/resourcehelpers.hpp>
#include <components/resource/resourcesystem.hpp>

#include "apps/openmw/mwbase/environment.hpp"

namespace sol
{
    template <>
    struct is_automagical<ESM::Enchantment> : std::false_type
    {
    };
}

namespace
{
    // Populates an enchantmnet struct from a Lua table.
    ESM::Enchantment tableToEnchantment(const sol::table& rec)
    {
        ESM::Enchantment enchantment;
        if (rec["template"] != sol::nil)
            enchantment = LuaUtil::cast<ESM::Enchantment>(rec["template"]);
        else
            enchantment.blank();

        if (rec["type"] != sol::nil)
        {
            int enchantmentType = rec["type"].get<int>();
            if (enchantmentType >= 0 && enchantmentType <= ESM::Enchantment::Last)
                enchantment.mData.mType = enchantmentType;
            else
                throw std::runtime_error("Invalid Enchantment Type provided: " + std::to_string(enchantmentType));
        }
        if (rec["cost"] != sol::nil)
            enchantment.mData.mCost = rec["cost"];
        if (rec["charge"] != sol::nil)
            enchantment.mData.mCharge = rec["charge"];
        if (rec["flags"] != sol::nil)
            enchantment.mData.mFlags = rec["flags"];
        if (rec["effects"] != sol::nil)
            enchantment.mEffects = rec["effects"]; // TO DO Unsure about effects

        return enchantment;
    }
}

namespace MWLua
{
    void addEnchantmentBindings(sol::table enchantment, const Context& context)
    {
        sol::state_view lua = context.sol();
        // TO DO: This already exists in magicbindings.cpp
        enchantment["TYPE"] = LuaUtil::makeStrictReadOnly(LuaUtil::tableFromPairs<std::string_view, int>(lua,
            {
                { "CastOnce", ESM::Enchantment::Type::CastOnce },
                { "CastOnStrike", ESM::Enchantment::Type::WhenStrikes },
                { "CastOnUse", ESM::Enchantment::Type::WhenUsed },
                { "ConstantEffect", ESM::Enchantment::Type::ConstantEffect },
            }));

        auto vfs = MWBase::Environment::get().getResourceSystem()->getVFS();

        addRecordFunctionBinding<ESM::Enchantment>(enchantment, context);
        enchantment["createRecordDraft"] = tableToEnchantment;

        sol::usertype<ESM::Enchantment> record = lua.new_usertype<ESM::Enchantment>("ESM3_Enchantment");
        record[sol::meta_function::to_string] = [](const ESM::Enchantment& rec) -> std::string {
            return "ESM3_Enchantment[" + rec.mId.toDebugString() + "]";
        };
        record["id"] = sol::readonly_property([](const ESM::Enchantment& rec) { return rec.mId.serializeText(); });
        enchantT["effects"]
            = sol::readonly_property([lua = state.lua_state()](const ESM::Enchantment& rec) -> sol::table {
                  return effectParamsListToTable(lua, rec.mEffects.mList);
              });
        record["type"] = sol::readonly_property([](const ESM::Enchantment& rec) -> int { return rec.mData.mType; });
        record["cost"] = sol::readonly_property([](const ESM::Enchantment& rec) -> int { return rec.mData.mCost; });
        record["charge"]
            = sol::readonly_property([](const ESM::Enchantment& rec) -> int { return rec.mData.mCharge; });
        record["autocalcFlag"] = sol::readonly_property(
            [](const ESM::Enchantment& rec) -> bool { return !!(rec.mData.mFlags & ESM::Enchantment::Autocalc); });

    }

}
