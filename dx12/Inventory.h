#pragma once
#include "InventoryItem.h"
#include <vector>
#include <memory>

class Inventory
{
public:
    // 所持金
    int gold = 0;

    // アイテムのインスタンスをポインタまたはスマートポインタで管理
    std::vector<std::unique_ptr<InventoryItem>> items;

    // 素材の管理（ひな形IDと数を直接持つ方が効率的）
    std::unordered_map<std::wstring, int> materials;

    /**
     * @brief アイテムのインスタンスを生成してインベントリに追加する
     * @param baseItemId ItemDataに登録されているひな形のID
     * @param allItems DXApplication::allItems_への参照
     */
    void AddItem(const std::wstring& baseItemId, const std::unordered_map<std::wstring, Item>& allItems)
    {
        auto it = allItems.find(baseItemId);
        if (it != allItems.end()) {
            const Item* base = &(it->second);

            if (base->type == ItemType::Material) {
                // 素材は InventoryItem にせず、数だけ増やす
                materials[baseItemId]++;
            }
            else {
                // 装備品/消耗品は InventoryItem のインスタンスを作成
                items.push_back(std::make_unique<InventoryItem>(base));
            }
        }
    }
};