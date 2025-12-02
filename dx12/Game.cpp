#include "Game.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <vector>

Game* Game::GetInstance()
{
	static Game instance;
	return &instance;
}

Unit* Game::GetUnit(const std::wstring& id)
{
	auto it = allUnits_.find(id);
	return (it != allUnits_.end()) ? &(it->second) : nullptr;
}

Enemy* Game::GetEnemy(const std::wstring& id)
{
	auto it = allEnemies_.find(id);
	return (it != allEnemies_.end()) ? &(it->second) : nullptr;
}

Item* Game::GetItem(const std::wstring& id)
{
	auto it = allItems_.find(id);
	return (it != allItems_.end()) ? &(it->second) : nullptr;
}

//=============================================================================
// Helper Functions
//=============================================================================

std::wstring Game::CleanValue(const std::wstring& str) {
	std::wstring cleaned = str;

	// 前後の空白文字を削除 (トリミング)
	const auto is_space = [](wchar_t c) { return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n'; };
	size_t first = cleaned.find_first_not_of(L" \t\r\n");
	if (std::wstring::npos == first) return L"";
	size_t last = cleaned.find_last_not_of(L" \t\r\n");
	cleaned = cleaned.substr(first, (last - first + 1));

	// 引用符 ("") があれば削除
	if (cleaned.length() >= 2 && cleaned.front() == L'"' && cleaned.back() == L'"') {
		cleaned = cleaned.substr(1, cleaned.length() - 2);
	}
	return cleaned;
}

std::wstring Utf8ToWString(const std::string& utf8_str)
{
	if (utf8_str.empty()) return L"";

	// MultiByteToWideCharを使用してUTF-8をwchar_tに変換
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), NULL, 0);
	if (size_needed <= 0) return L"";

	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

EnchantType Game::StringToEnchantType(const std::wstring& s) {
	if (s == L"SKILL_LV_UP") return EnchantType::SKILL_LV_UP;
	if (s == L"FIRE_RESISTANCE") return EnchantType::FIRE_RESISTANCE;
	if (s == L"POISON_RESISTANCE") return EnchantType::POISON_RESISTANCE;
	if (s == L"ATK_PER_LEVEL") return EnchantType::ATK_PER_LEVEL;
	// ... すべての EnchantType に対応する分岐を追加
	return EnchantType::NONE;
}

//=============================================================================
// Data Loading Functions
//=============================================================================

void Game::LoadUnitData(const std::wstring& filepath)
{
	std::ifstream file;
	file.open(filepath);

	if (!file.is_open()) {
		OutputDebugString(L"ERROR: Unit data file not found!\n");
		return;
	}

	std::string line_str; // stringで読み込む
	Unit currentUnit = {};
	bool inUnitBlock = false;
	bool readingDetail = false; // detailの複数行読み取り中フラグ
	std::wstring detailBuffer = L""; // detailの文字列を格納するバッファ

	while (std::getline(file, line_str)) {

		// UTF-8バイト列をワイド文字列に変換
		std::wstring line = Utf8ToWString(line_str);

		// BOM (\uFEFF) の削除
		if (!line.empty() && line[0] == L'\ufeff') {
			line.erase(0, 1);
		}

		// 行頭の空白を削除し、コメント行をチェック
		size_t first_char = line.find_first_not_of(L" \t");
		bool is_empty_or_comment = (first_char == std::wstring::npos || line[first_char] == L'#');
		std::wstring trimmedLine = (first_char == std::wstring::npos) ? L"" : line.substr(first_char);

		// ★ 複数行detailの読み取りロジック ★
		if (readingDetail) {

			// 現在の行の閉じ引用符の位置を探す
			size_t last_quote = line.find_last_of(L'"');
			// 行頭の引用符は無視するために、find_last_ofを使用
			// ただし、最後の行が " だけの場合もあるので、行全体で探す。

			if (last_quote != std::wstring::npos) {
				// 閉じ引用符が見つかった場合：detailの読み取りを終了
				// 閉じ引用符までの内容をバッファに追加
				detailBuffer += L"\r\n" + line.substr(0, last_quote);

				// detail値を確定。バッファには既に開始引用符がない状態
				// CleanValueは呼び出さない (detailBufferには引用符が残らないため)
				currentUnit.detail = detailBuffer;

				readingDetail = false;
				detailBuffer = L"";

				// 閉じ引用符の後に続く文字は、次のパースのためにスキップ
				continue;

			}
			else {
				// 閉じ引用符が見つからない場合：読み取りを継続
				detailBuffer += L"\r\n" + line; // 改行文字を保持して行全体を追加
				continue;
			}
		}

		if (is_empty_or_comment) continue;


		// --- ユニットブロック開始/終了のパース ---
		if (trimmedLine.rfind(L"unit ", 0) == 0) {
			if (inUnitBlock) {
				if (!currentUnit.id.empty()) {
					allUnits_[currentUnit.id] = currentUnit;
				}
			}
			std::wstring unitName = trimmedLine.substr(5);
			size_t open_brace = unitName.find(L'{');
			if (open_brace != std::wstring::npos) {
				unitName = unitName.substr(0, open_brace);
			}
			currentUnit = {};
			currentUnit.id = CleanValue(unitName);
			inUnitBlock = true;
			continue;
		}

		if (trimmedLine.rfind(L"}", 0) == 0 && inUnitBlock) {
			if (!currentUnit.id.empty()) {
				allUnits_[currentUnit.id] = currentUnit;
			}
			inUnitBlock = false;
			continue;
		}
		// -------------------------------------------------------------------

		// ブロック内のキー-値のペアをパース
		if (inUnitBlock) {
			// ★ 修正: ローカル変数を宣言し、誤った識別子を修正 ★
			size_t eq_pos = trimmedLine.find(L'=');
			if (eq_pos == std::wstring::npos) continue;

			std::wstring key = CleanValue(trimmedLine.substr(0, eq_pos));
			std::wstring value = trimmedLine.substr(eq_pos + 1);

			if (key == L"detail") {

				// 値の先頭の空白を削除
				size_t detail_start = value.find_first_not_of(L" \t");
				if (detail_start != std::wstring::npos) {
					value = value.substr(detail_start);
				}

				// 値が開始引用符で始まっているか確認
				if (value.front() == L'"') {
					// 最初の行で閉じ引用符を探す
					size_t last_quote = value.find_last_of(L'"');

					// last_quote != 0 のチェックだと、valueが "abc" の場合でも単一行とみなせる。
					if (last_quote != 0 && last_quote == value.length() - 1) {
						// 引用符が末尾にある場合 (単一行で完結)
						currentUnit.detail = CleanValue(value);
					}
					else {
						// 閉じ引用符が見つからない、または閉じ引用符が末尾にない場合 (複数行が期待される)
						detailBuffer = value.substr(1); // 開始引用符を除く
						readingDetail = true;
					}
				}
				else {
					// 引用符で囲まれていない場合は単一行として処理 (CleanValueが空白をトリミング)
					currentUnit.detail = CleanValue(value);
				}
			}
			else {
				value = CleanValue(value); // detail以外はCleanValueを適用

				try {
					if (key == L"name") currentUnit.name = value;
					else if (key == L"image") currentUnit.imagePath = value;
					else if (key == L"race") currentUnit.race = value;
					else if (key == L"sex") currentUnit.sex = value;
					else if (key == L"hp") currentUnit.hp = std::stoi(value);
					// ... (他の数値パースも続く) ...
					else if (key == L"mp") currentUnit.mp = std::stoi(value);
					else if (key == L"attack") currentUnit.attack = std::stoi(value);
					else if (key == L"defence") currentUnit.defence = std::stoi(value);
					else if (key == L"magic") currentUnit.magic = std::stoi(value);
					else if (key == L"mental") currentUnit.mental = std::stoi(value);
					else if (key == L"speed") currentUnit.speed = std::stoi(value);
					else if (key == L"father") currentUnit.fatherId = value;
					else if (key == L"mother") currentUnit.motherId = value;
					else if (key == L"level") currentUnit.level = std::stoi(value);
					else if (key == L"exp") currentUnit.exp = std::stoi(value);
					else if (key == L"resistance") {
						// resistance の特殊パース
						std::wstringstream rs(value);
						std::wstring resEntry;
						while (std::getline(rs, resEntry, L',')) {
							size_t star_pos = resEntry.find(L'*');
							if (star_pos != std::wstring::npos) {
								Resistance r = {};
								r.type = CleanValue(resEntry.substr(0, star_pos));
								try {
									r.value = std::stoi(CleanValue(resEntry.substr(star_pos + 1)));
									currentUnit.resistances.push_back(r);
								}
								catch (...) {
									OutputDebugString(L"WARNING: Failed to parse resistance value in unit data.\n");
								}
							}
						}
					}
				}
				catch (const std::exception& e) {
					std::wstring errorMsg = L"ERROR: Failed to parse numeric data for key: " + key + L" in unit: " + currentUnit.id + L"\n";
					OutputDebugString(errorMsg.c_str());
				}
			}
		}
	}

	if (inUnitBlock && !currentUnit.id.empty()) {
		allUnits_[currentUnit.id] = currentUnit;
	}
}

EquipSlot Game::StringToEquipSlot(const std::wstring& s) {
	if (s == L"Head") return EquipSlot::Head;
	if (s == L"Body") return EquipSlot::Body;
	if (s == L"Neck") return EquipSlot::Neck;
	if (s == L"Hand_Weapon") return EquipSlot::Hand_Weapon;
	if (s == L"Hand_Shield") return EquipSlot::Hand_Shield;
	if (s == L"Back") return EquipSlot::Back;
	if (s == L"Waist") return EquipSlot::Waist;
	if (s == L"Arm") return EquipSlot::Arm;
	if (s == L"Leg") return EquipSlot::Leg;

	// ★指を統一
	if (s == L"Finger") return EquipSlot::Finger;

	return EquipSlot::NONE;
}

void Game::LoadItemData(const std::wstring& filepath)
{
	std::wifstream file(filepath);
	if (!file.is_open()) {
		OutputDebugString(L"ERROR: Item data file not found!\n");
		return;
	}

	std::wstring line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == L'#') continue;

		std::wstringstream ss(line);
		std::wstring segment;
		std::vector<std::wstring> tokens;

		while (std::getline(ss, segment, L',')) {
			tokens.push_back(segment);
		}

		// 基本情報 (ID, Name, Type, Description, Price) は最低5列
		if (tokens.size() < 5) {
			OutputDebugString(L"WARNING: Item data line skipped due to missing essential columns (ID-Price).\n");
			continue;
		}

		try {
			Item item;
			item.id = tokens[0];
			item.name = tokens[1];
			item.description = tokens[3];
			item.price = std::stoi(tokens[4]);

			// 1. ItemType の決定と設定 (tokens[2])
			// Material と Consumable は EquipSlot::NONE を共有するため、最初のブロックで処理
			if (tokens[2] == L"Material" || tokens[2] == L"Consumable") {
				item.type = (tokens[2] == L"Material") ? ItemType::Material : ItemType::Consumable;
				item.equipSlot = EquipSlot::NONE;

				if (item.type == ItemType::Consumable) {
					// 消耗品は基本5列 + 消耗品2列 (計7列) が必要
					if (tokens.size() < 7) {
						OutputDebugString(L"WARNING: Consumable item skipped (missing effect columns).\n");
						continue; // 必要な列がない場合はスキップ
					}
					// 効果情報 (tokens[5], tokens[6])
					item.targetType = tokens[5];
					item.effectValue = std::stoi(tokens[6]);
				}
			}
			else if (tokens[2] == L"Weapon" || tokens[2] == L"Armor") {
				// 装備品は全ての列 (11列: 基本5 + スロット1 + 装備4 + エンチャント1) が必要
				if (tokens.size() < 11) {
					OutputDebugString(L"WARNING: Equipment item skipped (missing required columns).\n");
					continue;
				}

				item.type = (tokens[2] == L"Weapon") ? ItemType::Weapon : ItemType::Armor;

				// スロット、素材、ボーナスステータス
				item.equipSlot = StringToEquipSlot(tokens[5]);
				item.matNeededId = tokens[6];
				item.requiredMats = std::stoi(tokens[7]);
				item.attackBonus = std::stoi(tokens[8]);
				item.hpBonus = std::stoi(tokens[9]);

				// エンチャントリストのパース (tokens[10])
				std::wstring enchantListStr = tokens[10];
				std::wstringstream enchantListSS(enchantListStr);
				std::wstring enchantEntry;

				// パイプ '|' で個々のエンチャントのエントリを分割
				while (std::getline(enchantListSS, enchantEntry, L'|')) {
					std::wstringstream enchantEntrySS(enchantEntry);
					std::wstring enchantToken;
					std::vector<std::wstring> enchantTokens;

					// コロン ':' で Type, Value, TargetID を分割
					while (std::getline(enchantEntrySS, enchantToken, L':')) {
						enchantTokens.push_back(enchantToken);
					}

					if (enchantTokens.size() == 3) {
						Enchantment ench = {};
						ench.type = StringToEnchantType(enchantTokens[0]);
						ench.value = std::stoi(enchantTokens[1]);
						ench.targetId = enchantTokens[2]; // スキルIDなどを格納
						item.enchantments.push_back(ench);
					}
					else {
						OutputDebugString(L"WARNING: Invalid enchantment format.\n");
					}
				}
			}
			else {
				OutputDebugString(L"WARNING: Unknown item type found.\n");
				continue;
			}

			allItems_[item.id] = item;

		}
		catch (const std::exception& e) {
			std::wstring errorMsg = L"ERROR: Failed to parse item data for ID: " + tokens[0] + L"\n";
			OutputDebugString(errorMsg.c_str());
		}
	}
}

void Game::LoadEnemyData(const std::wstring& filepath)
{
	std::wifstream file(filepath);

	if (!file.is_open()) {
		OutputDebugString(L"ERROR: Enemy data file not found!\n");
		return;
	}

	std::wstring line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == L'#') continue;

		std::wstringstream ss(line);
		std::wstring segment;
		std::vector<std::wstring> tokens;

		while (std::getline(ss, segment, L',')) {
			tokens.push_back(segment);
		}

		if (tokens.size() < 7) {
			OutputDebugString(L"WARNING: Enemy data line skipped due to missing columns.\n");
			continue;
		}

		try {
			// 敵 Enemy クラスのインスタンスを生成
			Enemy enemy;
			enemy.id = tokens[0];
			enemy.name = tokens[1];
			enemy.imagePath = tokens[2];
			enemy.maxHp = std::stoi(tokens[3]);
			enemy.attack = std::stoi(tokens[4]);
			enemy.baseExp = std::stoi(tokens[5]);

			// ドロップリストのパース (tokens[6])
			std::wstring dropListStr = tokens[6];
			std::wstringstream dropListSS(dropListStr);
			std::wstring dropEntry;

			while (std::getline(dropListSS, dropEntry, L'|')) {
				std::wstringstream dropEntrySS(dropEntry);
				std::wstring itemToken;
				std::vector<std::wstring> itemTokens;

				while (std::getline(dropEntrySS, itemToken, L':')) {
					itemTokens.push_back(itemToken);
				}

				if (itemTokens.size() == 2) {
					// DropInfo 構造体のインスタンスを生成
					DropInfo info;
					info.itemId = itemTokens[0];
					info.dropRate = std::stof(itemTokens[1]);
					enemy.drops.push_back(info); // Enemy オブジェクトの vector に追加
				}
				else {
					OutputDebugString(L"WARNING: Invalid drop format in EnemyData.\n");
				}
			}

			allEnemies_[enemy.id] = enemy; // マップに Enemy オブジェクトを格納

		}
		catch (const std::exception& e) {
			std::wstring errorMsg = L"ERROR: Failed to parse enemy data for ID: " + tokens[0] + L"\n";
			OutputDebugString(errorMsg.c_str());
		}
	}
}