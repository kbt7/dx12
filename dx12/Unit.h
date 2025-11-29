#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// 属性耐性データ
struct Resistance
{
    std::wstring type; // 例: "fire", "water"
    int value;         // 例: 3, -102
};

class Unit
{
public:
    std::wstring id;            // ユニットID (ユニット名から取得)
    std::wstring name;
    std::wstring imagePath;
    std::wstring race;
    std::wstring sex;
    int hp = 0;
    int mp = 0;
    int attack = 0;
    int defence = 0;
    int magic = 0;
    int mental = 0;
    int speed = 0;
    std::wstring fatherId;      // "@"は空文字として扱う
    std::wstring motherId;      // "@"は空文字として扱う
    std::wstring detail;        // 詳細説明文
    int level = 1;
    int exp = 0;

    std::vector<Resistance> resistances; // 属性耐性のリスト
};