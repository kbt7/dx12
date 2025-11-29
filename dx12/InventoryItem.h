#pragma once
#include "Item.h" // Item クラスの定義をインクルード

class InventoryItem
{
public:
    // ★Item のひな形データへのポインタ
    // どの種類のアイテムであるかを静的に参照する
    const Item* baseItem = nullptr;

    // ★アイテムの個体差を示すユニークな情報
    // 1. エンチャントのリスト（ランダムに付与される特性）
    std::vector<Enchantment> uniqueEnchantments;

    // 2. アイテムの個体名
    std::wstring uniqueName = L"";

    // 3. その他ユニークな情報（耐久度、作成者など）
    int durability = 100;

    // コンストラクタ: ひな形を基にインスタンスを作成
    InventoryItem(const Item* base) : baseItem(base)
    {
        // ここでランダムなエンチャントを付与するロジックを実行する
        InitializeUniqueProperties();
    }

    // デフォルトコンストラクタは禁止 (必ずひな形が必要)
    InventoryItem() = delete;

private:
    void InitializeUniqueProperties()
    {
        // 例: ベースアイテムが装備品の場合、ランダムでエンチャントを追加
        if (baseItem && (baseItem->type == ItemType::Weapon || baseItem->type == ItemType::Armor))
        {
            // 基礎エンチャント（ひな形に定義されているもの）をコピー
            uniqueEnchantments = baseItem->enchantments;

            // ランダムで追加エンチャントを付与するロジック（例: 20%の確率でスキル+1を付与）
            if (rand() % 100 < 20) {
                Enchantment bonusEnch = {};
                bonusEnch.type = EnchantType::SKILL_LV_UP;
                bonusEnch.value = 1;
                bonusEnch.targetId = L"SWORD_MASTER";
                uniqueEnchantments.push_back(bonusEnch);
                uniqueName = L"マスターの " + baseItem->name;
            }
            else {
                uniqueName = baseItem->name;
            }
        }
        else
        {
            uniqueName = baseItem->name;
        }
    }
};