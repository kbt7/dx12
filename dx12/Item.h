#pragma once
#include<string>
#include<unordered_map>

enum class EnchantType {
    NONE,
    SKILL_LV_UP,        // 特定のスキルレベル上昇
    FIRE_RESISTANCE,    // 火炎耐性
    POISON_RESISTANCE,  // 毒耐性
    ATK_PER_LEVEL,      // レベルに応じた攻撃力ボーナス
    // ... ユニークな効果をここに追加 ...
};

// アイテムが持つエンチャントのデータ構造
struct Enchantment
{
    EnchantType type = EnchantType::NONE;
    int value = 0; // 効果値 (例: スキルレベル+1, 耐性+10%, ATK+2/Lv)
    std::wstring targetId; // 効果の対象ID (例: 対象スキルID, 属性IDなど)
};

enum class ItemType {
    Material,   // 素材
    Weapon,     // 武器
    Armor,      // 防具
    Consumable  // ★消耗品 (追加)
};

enum class EquipSlot {
    NONE,           // 装備不可のアイテム（素材、消耗品など）
    Head,           // 頭
    Body,           // 胴体
    Neck,           // 首 (ネックレス)
    Hand_Weapon,    // 手 (武器)
    Hand_Shield,    // 手 (盾/防具)
    Back,           // 背中 (マントなど)
    Waist,          // 腰 (ベルトなど)
    Arm,            // 腕 (小手など)
    Leg,            // 足 (靴など)
    Finger,         // ★指 (左右統一)
    // Finger_L と Finger_R は削除
};

struct Item
{

    EquipSlot equipSlot = EquipSlot::NONE;

    std::vector<Enchantment> enchantments;

    std::wstring id;            // アイテムID (例: MATERIAL_IRON)
    std::wstring name;          // アイテム名
    ItemType type;              // アイテム種別
    std::wstring description;   // 説明文

    int price = 0;              // ★[新] アイテムの売買価格 (共通)

    // 素材の場合
    int requiredMats;           // 装備作成に必要な素材量 (装備品の場合のみ使用)
    std::wstring matNeededId;   // 装備作成に必要な素材のID (装備品の場合のみ使用)

    // 装備品の場合
    int attackBonus;            // 攻撃力ボーナス
    int hpBonus;                // HPボーナス
    // ... その他ボーナスステータス
    int effectValue = 0;        // 回復量、バフの数値など (消耗品の場合)
    std::wstring targetType;    // 効果の対象 (HP_RECOVERY, MP_RECOVERY, BUFF_ATK, etc.)
};

// 全アイテムデータを格納するマップ
std::unordered_map<std::wstring, Item> allItems_;