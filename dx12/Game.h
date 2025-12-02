#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include "Unit.h"
#include "Enemy.h"
#include "Item.h"

class Game
{
public:
	static Game* GetInstance();

	void LoadUnitData(const std::wstring& filepath);
	void LoadEnemyData(const std::wstring& filepath);
	void LoadItemData(const std::wstring& filepath);

	const std::unordered_map<std::wstring, Unit>& GetUnits() const { return allUnits_; }
	const std::unordered_map<std::wstring, Enemy>& GetEnemies() const { return allEnemies_; }
	const std::unordered_map<std::wstring, Item>& GetItems() const { return allItems_; }

	Unit* GetUnit(const std::wstring& id);
	Enemy* GetEnemy(const std::wstring& id);
	Item* GetItem(const std::wstring& id);

private:
	Game() = default;
	~Game() = default;
	Game(const Game&) = delete;
	void operator=(const Game&) = delete;

	// Helper functions for parsing
	static std::wstring CleanValue(const std::wstring& str);
	static EquipSlot StringToEquipSlot(const std::wstring& s);
	static EnchantType StringToEnchantType(const std::wstring& s);

	std::unordered_map<std::wstring, Unit> allUnits_;
	std::unordered_map<std::wstring, Enemy> allEnemies_;
	std::unordered_map<std::wstring, Item> allItems_;
};
