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

        if (rec["mwscript"] != sol::nil)
        {
            std::string_view scriptId = rec["mwscript"].get<std::string_view>();
            weapon.mScript = ESM::RefId::deserializeText(scriptId);
        }

        if (rec["type"] != sol::nil)
        {
            int enchantmentType = rec["type"].get<int>();
            if (enchantmentType >= 0 && enchantmentType <= ESM::Enchantment::Last)
                enchantment.mData.mType = enchantmentType;
            else
                throw std::runtime_error("Invalid Enchantment Type provided: " + std::to_string(enchantmentType));
        }
        if (rec["cost"] != sol::nil)
            enchantment.mData.mWeight = rec["weight"];
        if (rec["charge"] != sol::nil)
            enchantment.mData.mValue = rec["value"];
        if (rec["flags"] != sol::nil)
            enchantment.mData.mHealth = rec["health"];
        if (rec["effects"] != sol::nil)
            enchantment.mData.mSpeed = rec["speed"];
        if (rec["deleted"] != sol::nil)
            enchantment.mData.mReach = rec["reach"];
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

        sol::usertype<ESM::Weapon> record = lua.new_usertype<ESM::Weapon>("ESM3_Weapon");
        record[sol::meta_function::to_string]
            = [](const ESM::Weapon& rec) -> std::string { return "ESM3_Weapon[" + rec.mId.toDebugString() + "]"; };
        record["id"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> std::string { return rec.mId.serializeText(); });
        record["name"] = sol::readonly_property([](const ESM::Weapon& rec) -> std::string { return rec.mName; });
        addModelProperty(record);
        record["icon"] = sol::readonly_property([vfs](const ESM::Weapon& rec) -> std::string {
            return Misc::ResourceHelpers::correctIconPath(rec.mIcon, vfs);
        });
        record["enchant"] = sol::readonly_property(
            [](const ESM::Weapon& rec) -> sol::optional<std::string> { return LuaUtil::serializeRefId(rec.mEnchant); });
        record["mwscript"] = sol::readonly_property(
            [](const ESM::Weapon& rec) -> sol::optional<std::string> { return LuaUtil::serializeRefId(rec.mScript); });
        record["isMagical"] = sol::readonly_property(
            [](const ESM::Weapon& rec) -> bool { return rec.mData.mFlags & ESM::Weapon::Magical; });
        record["isSilver"] = sol::readonly_property(
            [](const ESM::Weapon& rec) -> bool { return rec.mData.mFlags & ESM::Weapon::Silver; });
        record["weight"] = sol::readonly_property([](const ESM::Weapon& rec) -> float { return rec.mData.mWeight; });
        record["value"] = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mValue; });
        record["type"] = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mType; });
        record["health"] = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mHealth; });
        record["speed"] = sol::readonly_property([](const ESM::Weapon& rec) -> float { return rec.mData.mSpeed; });
        record["reach"] = sol::readonly_property([](const ESM::Weapon& rec) -> float { return rec.mData.mReach; });
        record["enchantCapacity"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> float { return rec.mData.mEnchant * 0.1f; });
        record["chopMinDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mChop[0]; });
        record["chopMaxDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mChop[1]; });
        record["slashMinDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mSlash[0]; });
        record["slashMaxDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mSlash[1]; });
        record["thrustMinDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mThrust[0]; });
        record["thrustMaxDamage"]
            = sol::readonly_property([](const ESM::Weapon& rec) -> int { return rec.mData.mThrust[1]; });
    }

}
