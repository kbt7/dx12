#pragma once
#include<string>
#include<unordered_map>

struct DropInfo
{
    std::wstring itemId;    // ドロップアイテムID (ItemData.txtのIDと紐づく)
    float dropRate;         // ドロップ確率 (0.0f - 1.0f)
};

struct Enemy
{
    std::wstring id;            // 敵ID
    std::wstring name;          // 敵名
    std::wstring imagePath;     // 戦闘時の画像パス
    int maxHp;                  // 最大HP
    int attack;                 // 攻撃力
    int baseExp;                // 基本経験値 (勝利時にパーティに配分)
    std::vector<DropInfo> drops;
};

// 全敵データを格納するマップ
std::unordered_map<std::wstring, Enemy> allEnemies_;