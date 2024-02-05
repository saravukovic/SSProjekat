#include "../inc/linker.h"

Linker::Linker(list<string> input, string output, list<string> placement)
{
	for(string i : input){
		inputFileNameArrLinker.push_back(i);
	}

	outputFileNameLinker = output;

	for(string p : placement){
		this->placement.push_back(p);
	}
	pc = 0;
}

Linker::~Linker()
{
}

void Linker::link()
{
	string path = "./test/";
	outputFileLinker = path + outputFileNameLinker;

	for (string s : inputFileNameArrLinker) {
		inputFilesLinker.push_back(path + s);
	}

	string place = "-place=";

	for (string p : placement) {
		p.erase(0, place.size());
		string sec, adr;
		size_t i = p.find("@");
		sec = p.substr(0, i);
		p.erase(0, i + 1); //delete delimiter also (,)
		adr = p;

		unsigned int val = 0;
		if (adr[0] == '0' and adr[1] == 'x') {
			adr.erase(0, 2);
			val = stoul(adr, 0, 16);
		}
		else
		{
			val = stoul(adr, 0, 10);
		}

		placements.insert({ sec, val });

	}

	for (string input : inputFilesLinker) {
		parseBinaryInput((input).substr(0, input.size() - 1) + "dat");
	}

	SectionDefinition::resetSectionID();
	SymbolDefinition::resetSymID();

	createSectionTable();
	createSymbolTable();
	resolveAllSymbols();
	createHEX();
	createBinary();
}

void Linker::parseBinaryInput(string inputFile)
{
	ifstream file(inputFile, ios::binary);

	if (!file.is_open()) {
		cout << "Greska! Dokument sa imenom " << inputFile << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	SectionDefinition::resetSectionID();
	SymbolDefinition::resetSymID();

	size_t symbolTableSize = 0;
	file.read((char*)&symbolTableSize, sizeof(size_t));

	map<string, SymbolDefinition> symTable;
	symbolTables.insert({ inputFile, symTable });

	while (symbolTableSize) {
		string symName, secName;
		int secNum = 0, symID = 0;
		unsigned int adress = 0;
		SymbolBinding bind = GLOBAL;
		SymbolType type = NOTYPE;
		bool isExtern = false, isDefined = false;
		size_t symNameSize = 0, secNameSize = 0;

		file.read((char*)&symNameSize, sizeof(symNameSize));
		symName.resize(symNameSize);
		file.read((char*)symName.c_str(), symNameSize);

		file.read((char*)&symID, sizeof(symID));
		file.read((char*)&adress, sizeof(adress));
		file.read((char*)&secNum, sizeof(secNum));

		file.read((char*)&secNameSize, sizeof(secNameSize));
		secName.resize(secNameSize);
		file.read((char*)secName.c_str(), secNameSize);

		file.read((char*)&bind, sizeof(bind));
		file.read((char*)&type, sizeof(type));
		file.read((char*)&isExtern, sizeof(isExtern));
		file.read((char*)&isDefined, sizeof(isDefined));

		symbolTables.at(inputFile).insert({ symName, SymbolDefinition(symName, secName, secNum, type, bind, adress, isExtern) });
		symbolTables.at(inputFile).at(symName).setSymID(symID);

		symbolTableSize--;
	}

	size_t sectionTableSize = 0;
	file.read((char*)&sectionTableSize, sizeof(sectionTableSize));

	map<string, SectionDefinition*> secTable;
	sectionTables.insert({ inputFile, secTable });

	while (sectionTableSize) {
		string secName;
		int secID = 0, secSize = 0;
		size_t secNameSize = 0;

		file.read((char*)&secNameSize, sizeof(secNameSize));
		secName.resize(secNameSize);
		file.read((char*)secName.c_str(), secNameSize);

		file.read((char*)&secID, sizeof(secID));
		file.read((char*)&secSize, sizeof(secSize));

		SectionDefinition* section = new SectionDefinition(secName);

		section->setSectionSize(secSize);
		section->setSectionID(secID);

		size_t contentSize = 0;
		file.read((char*)&contentSize, sizeof(contentSize));

		while (contentSize) {
			char content = 0;

			file.read((char*)&content, sizeof(content));
			section->addSectionBinaryContent(content);
			contentSize--;
		}

		size_t relTableSize = 0;
		file.read((char*)&relTableSize, sizeof(relTableSize));

		while (relTableSize) {
			string type;
			unsigned int offset = 0;
			int symbolId = 0, addend = 0;
			size_t typeSize = 0;

			file.read((char*)&offset, sizeof(offset));

			file.read((char*)&typeSize, sizeof(typeSize));
			type.resize(typeSize);
			file.read((char*)type.c_str(), typeSize);
			file.read((char*)&symbolId, sizeof(symbolId));
			file.read((char*)&addend, sizeof(addend));

			section->addRelocationTableEntry(RelocationTableEntry(type, secName, offset, symbolId, addend));
			relTableSize--;
		}

		sectionTables.at(inputFile).insert({ secName, section });

		sectionTableSize--;
	}
}

void Linker::createSectionTable()
{
	for (string file : inputFilesLinker)
	{
		map<string, SectionDefinition*> section = sectionTables.at(file.substr(0, file.size() - 1) + "dat");

		for (auto& sec : section) {
			string secName = sec.first;
			SectionDefinition* s = sec.second;
			if (placements.count(secName) != 0) {

				if (sectionTable.count(secName) == 0) {

					unsigned int adr = placements.at(secName);

					s->setSectionOffset(adr);

					SectionDefinition* linkerSection = new SectionDefinition(secName, adr, s->getSectionSize());

					sectionTable.insert({ secName, linkerSection });

					for (char content : s->getContent()) {
						linkerSection->addSectionBinaryContent(content);
					}

				}
				else
				{
					SectionDefinition* secInTable = sectionTable.at(secName);
					s->addSectionOffset(secInTable->getSectionSize());
					secInTable->addSectionSize(s->getSectionSize());
					for (char content : s->getContent()) {
						secInTable->addSectionBinaryContent(content);
					}
				}
			}
		}
	}
	//proveri jel ima preklapanja i pronadji startnu adresu
	unsigned int startAdress = 0;
	for (auto& sec1 : sectionTable) {
		unsigned int sec1End = sec1.second->getSectionOffset() + sec1.second->getSectionSize();
		for (auto& sec2 : sectionTable) {
			unsigned int sec2End = sec2.second->getSectionOffset() + sec2.second->getSectionSize();
			if (sec1 != sec2 &&
				max(sec1.second->getSectionOffset(), sec2.second->getSectionOffset()) <
				min(sec1End, sec2End)) {
				cout << "Greska, sekcije " << sec1.first << " i " << sec2.first << " se preklapaju." << endl;
				exit(-1);
			}
		}
		if (sec1End > startAdress) startAdress = sec1End;
	}

	for (string file : inputFilesLinker)
	{
		map<string, SectionDefinition*> section = sectionTables.at(file.substr(0, file.size() - 1) + "dat");

		for (auto& sec : section) {
			string secName = sec.first;
			SectionDefinition* s = sec.second;
			if (placements.count(secName) == 0) {
				if (sectionTable.count(secName) == 0) {
					unsigned int adr = startAdress;

					s->setSectionOffset(adr);

					SectionDefinition* linkerSection = new SectionDefinition(secName, adr, s->getSectionSize());

					sectionTable.insert({ secName, linkerSection });

					for (char content : s->getContent()) {
						linkerSection->addSectionBinaryContent(content);
					}
				}
				else
				{
					SectionDefinition* secInTable = sectionTable.at(secName);
					s->setSectionOffset(secInTable->getSectionOffset() + secInTable->getSectionSize());
					secInTable->addSectionSize(s->getSectionSize());

					for (char content : s->getContent()) {
						secInTable->addSectionBinaryContent(content);
					}
				}
				startAdress += s->getSectionSize();
			}
		}
	}

	for (auto& entry : sectionTable) {
		sortedListOfSections.push_back(entry.second);
	}

	sortedListOfSections.sort([](SectionDefinition* a, SectionDefinition* b) { return a->getSectionOffset() < b->getSectionOffset(); });

}

void Linker::createSymbolTable()
{
	for (auto& sec : sectionTable) {
		SectionDefinition* section = sec.second;
		symbolTable.insert({ sec.first, SymbolDefinition(sec.first, sec.first, section->getSectionID(), SECTION, LOCAL, section->getSectionOffset(), false) });
	}

	for (string file : inputFilesLinker)
	{
		map<string, SymbolDefinition> symTable = symbolTables.at(file.substr(0, file.size() - 1) + "dat");
		map<string, SectionDefinition*> originalSection = sectionTables.at(file.substr(0, file.size() - 1) + "dat");

		for (auto& symbol : symTable) {
			SymbolDefinition sym = symbol.second;

			if (sym.getSymType() != SECTION && sym.getSymbolBinding() != LOCAL) {

				if (sym.isSymbolExtern()) {
					if (symbolTable.count(symbol.first) == 0) undefinedSymbols.insert({ symbol.first, sym });
					continue;
				}
				else if (symbolTable.count(symbol.first))
				{
					cout << "Greska, simbol " << symbol.first << " je vec definisan." << endl;
					exit(-1);
				}

				SectionDefinition* section = originalSection.at(sym.getSymbolsSection());

				symbolTable.insert({ symbol.first, SymbolDefinition(symbol.first, sym.getSymbolsSection(), section->getSectionID(), NOTYPE, GLOBAL, section->getSectionOffset() + sym.getSymAdress(), false) });

				if (undefinedSymbols.count(symbol.first))
				{
					undefinedSymbols.erase(symbol.first);
				}

			}
		}
	}

	if (undefinedSymbols.size()) {
		cout << "Greska! Postoje nedefinisani symboli." << endl;
		exit(-1);
	}

}

void Linker::resolveAllSymbols()
{
	for (string file : inputFilesLinker)
	{

		for (auto& secEntry : sectionTables.at(file.substr(0, file.size() - 1) + "dat")) {

			SectionDefinition* assemblerSection = secEntry.second;
			string symbol;

			for (RelocationTableEntry rel : assemblerSection->getRelTable()) {

				//po imenu pronadji koji symbol ima relokacioni zapis jer su se identifikatori promenili
				for (auto& symEntry : symbolTables.at(file.substr(0, file.size() - 1) + "dat")) {
					if (rel.getSymbolId() == symEntry.second.getSymID()) {
						symbol = symEntry.first;
						break;
					}
				}

				int index = assemblerSection->getSectionOffset() - sectionTable.at(secEntry.first)->getSectionOffset() + rel.getRelOffset();
				unsigned int val = symbolTable.at(symbol).getSymAdress() + rel.getAddend();

				int i = 0;

				while (i < 4) {
					sectionTable.at(secEntry.first)->setSectionBinaryContent(index + i, val);
					val >>= 8;
					i++;
				}

			}


		}
	}

}

void Linker::createHEX()
{
	ofstream file(outputFileLinker);

	if (!file.is_open()) {
		cout << "Greska! Dokument sa imenom " << outputFileNameLinker << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	int i = 0;

	for (SectionDefinition* section : sortedListOfSections) {

		unsigned int adress = section->getSectionOffset();

		for (char c : section->getContent()) {
			if (i % 8 == 0) {
				file << setw(4) << setfill('0') << hex << adress << " : ";
			}

			file << setw(2) << setfill('0') << hex << (c & 0xFF) << ((i % 8 != 7) ? " " : "\n");

			adress++;
			i++;
		}
	}
}

void Linker::createBinary()
{
	ofstream binFile(outputFileLinker.substr(0, outputFileLinker.size() - 3) + "dat", ios::binary);

	if (!binFile.is_open()) {
		cout << "Greska! Dokument sa imenom " << outputFileNameLinker << " nije moguce otvoriti" << endl;
		exit(-1);
	}

	size_t secSize = sectionTable.size();
	binFile.write((char*)&secSize, sizeof(secSize));

	for (auto& entry : sectionTable) {
		SectionDefinition* section = entry.second;

		list<char> content = section->getContent();
		unsigned int adress = section->getSectionOffset();
		size_t contentSize = content.size();

		binFile.write((char*)&adress, sizeof(adress));
		binFile.write((char*)&contentSize, sizeof(contentSize));
		for (char iter : content) {
			binFile.write((char*)&iter, sizeof(iter));
		}
	}
}
