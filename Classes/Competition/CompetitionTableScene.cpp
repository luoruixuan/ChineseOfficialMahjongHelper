﻿#include "CompetitionTableScene.h"
#include "Competition.h"
#include "../common.h"
#include "../widget/AlertView.h"

USING_NS_CC;

CompetitionTableScene *CompetitionTableScene::create(const std::shared_ptr<CompetitionData> &competitionData, unsigned currentRound) {
    CompetitionTableScene *ret = new (std::nothrow) CompetitionTableScene();
    if (ret != nullptr && ret->initWithData(competitionData, currentRound)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CompetitionTableScene::initWithData(const std::shared_ptr<CompetitionData> &competitionData, unsigned currentRound) {
    if (UNLIKELY(!BaseScene::initWithTitle(Common::format<256>("%s第%u轮", competitionData->name.c_str(), currentRound + 1)))) {
        return false;
    }

    _competitionData = competitionData;
    _currentRound = currentRound;
    _competitionTables = &_competitionData->rounds[currentRound].tables;

    if (_competitionTables->empty()) {
        rankBySerial();
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 排列座位按钮
    ui::Button *button = ui::Button::create("source_material/btn_square_highlighted.png", "source_material/btn_square_selected.png");
    this->addChild(button);
    button->setScale9Enabled(true);
    button->setContentSize(Size(55.0f, 20.0f));
    button->setTitleFontSize(12);
    button->setTitleText("排列座位");
    button->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height - 70.0f));
    button->addClickEventListener(std::bind(&CompetitionTableScene::onRankButton, this, std::placeholders::_1));

    // 列宽
    _colWidth[0] = visibleSize.width * 0.1f;
    _colWidth[1] = visibleSize.width * 0.1f;
    _colWidth[2] = visibleSize.width * 0.1f;
    _colWidth[3] = visibleSize.width * 0.2f;
    _colWidth[4] = visibleSize.width * 0.15f;
    _colWidth[5] = visibleSize.width * 0.15f;
    _colWidth[6] = visibleSize.width * 0.2f;

    // 中心位置
    _posX[0] = _colWidth[0] * 0.5f;
    _posX[1] = _posX[0] + _colWidth[0] * 0.5f + _colWidth[1] * 0.5f;
    _posX[2] = _posX[1] + _colWidth[1] * 0.5f + _colWidth[2] * 0.5f;
    _posX[3] = _posX[2] + _colWidth[2] * 0.5f + _colWidth[3] * 0.5f;
    _posX[4] = _posX[3] + _colWidth[3] * 0.5f + _colWidth[4] * 0.5f;
    _posX[5] = _posX[4] + _colWidth[4] * 0.5f + _colWidth[5] * 0.5f;
    _posX[6] = _posX[5] + _colWidth[5] * 0.5f + _colWidth[6] * 0.5f;

    const char *titleTexts[] = { "桌号", "座次", "编号", "选手姓名", "标准分", "比赛分" };
    for (int i = 0; i < 6; ++i) {
        Label *label = Label::createWithSystemFont(titleTexts[i], "Arail", 12);
        label->setColor(Color3B::BLACK);
        this->addChild(label);
        label->setPosition(Vec2(origin.x + _posX[i], visibleSize.height - 100.0f));
    }

    cw::TableView *tableView = cw::TableView::create();
    tableView->setContentSize(Size(visibleSize.width, visibleSize.height - 115.0f));
    tableView->setDelegate(this);
    tableView->setDirection(ui::ScrollView::Direction::VERTICAL);
    tableView->setVerticalFillOrder(cw::TableView::VerticalFillOrder::TOP_DOWN);

    tableView->setScrollBarPositionFromCorner(Vec2(5, 2));
    tableView->setScrollBarWidth(4);
    tableView->setScrollBarOpacity(0x99);
    tableView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    tableView->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f - 55.0f));
    tableView->reloadData();
    _tableView = tableView;
    this->addChild(tableView);

    return true;
}

ssize_t CompetitionTableScene::numberOfCellsInTableView(cw::TableView *table) {
    return _competitionTables->size();
}

cocos2d::Size CompetitionTableScene::tableCellSizeForIndex(cw::TableView *table, ssize_t idx) {
    return Size(0, 120);
}

cw::TableViewCell *CompetitionTableScene::tableCellAtIndex(cw::TableView *table, ssize_t idx) {
    typedef cw::TableViewCellEx<Label *, Label *[4], Label *[4], Label *[4], Label *[4], ui::Button *, LayerColor *[2]> CustomCell;
    CustomCell *cell = (CustomCell *)table->dequeueCell();

    if (cell == nullptr) {
        cell = CustomCell::create();

        CustomCell::ExtDataType &ext = cell->getExtData();
        Label *&tableLabel = std::get<0>(ext);
        Label *(&serialLabels)[4] = std::get<1>(ext);
        Label *(&nameLabels)[4] = std::get<2>(ext);
        Label *(&standardLabels)[4] = std::get<3>(ext);
        Label *(&competitionLabels)[4] = std::get<4>(ext);
        ui::Button *&button = std::get<5>(ext);
        LayerColor *(&layerColors)[2] = std::get<6>(ext);

        Size visibleSize = Director::getInstance()->getVisibleSize();

        layerColors[0] = LayerColor::create(Color4B(0xC0, 0xC0, 0xC0, 0x10), visibleSize.width, 119);
        cell->addChild(layerColors[0]);

        layerColors[1] = LayerColor::create(Color4B(0x80, 0x80, 0x80, 0x10), visibleSize.width, 119);
        cell->addChild(layerColors[1]);

        tableLabel = Label::createWithSystemFont("", "Arail", 12);
        tableLabel->setColor(Color3B::BLACK);
        cell->addChild(tableLabel);
        tableLabel->setPosition(Vec2(_posX[0], 60.0f));

        static const char *seatText[] = { "东", "南", "西", "北" };
        for (int i = 0; i < 4; ++i) {
            Label *label = Label::createWithSystemFont(seatText[i], "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[1], (float)(105 - i * 30)));

            label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[2], (float)(105 - i * 30)));
            serialLabels[i] = label;

            label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[3], (float)(105 - i * 30)));
            nameLabels[i] = label;

            label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[4], (float)(105 - i * 30)));
            standardLabels[i] = label;

            label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[5], (float)(105 - i * 30)));
            competitionLabels[i] = label;
        }

        button = ui::Button::create("source_material/btn_square_highlighted.png", "source_material/btn_square_selected.png");
        cell->addChild(button);
        button->setScale9Enabled(true);
        button->setContentSize(Size(_colWidth[6] - 8, 20.0f));
        button->setTitleFontSize(12);
        button->setTitleText("登记成绩");
        button->setPosition(Vec2(_posX[6], 60.0f));
        button->addClickEventListener(std::bind(&CompetitionTableScene::onRecordButton, this, std::placeholders::_1));
        Common::scaleLabelToFitWidth(button->getTitleLabel(), _colWidth[6] - 10);
    }

    const CustomCell::ExtDataType ext = cell->getExtData();
    Label *tableLabel = std::get<0>(ext);
    Label *const (&serialLabels)[4] = std::get<1>(ext);
    Label *const (&nameLabels)[4] = std::get<2>(ext);
    Label *const (&standardLabels)[4] = std::get<3>(ext);
    Label *const (&competitionLabels)[4] = std::get<4>(ext);
    ui::Button *button = std::get<5>(ext);
    LayerColor *const (&layerColors)[2] = std::get<6>(ext);

    layerColors[0]->setVisible(!(idx & 1));
    layerColors[1]->setVisible(!!(idx & 1));

    tableLabel->setString(std::to_string(idx + 1));

    const std::vector<CompetitionPlayer> &players = _competitionData->players;

    for (int i = 0; i < 4; ++i) {
        const CompetitionTable &currentTable = _competitionTables->at(idx);
        ptrdiff_t playerIndex = currentTable.player_indices[i];
        if (playerIndex == INVALID_INDEX) {
            serialLabels[i]->setString("");
            nameLabels[i]->setString("");
            standardLabels[i]->setString("");
            competitionLabels[i]->setString("");
        }
        else {
            const CompetitionPlayer *player = &players[currentTable.player_indices[i]];
            serialLabels[i]->setString(std::to_string(player->serial));
            nameLabels[i]->setString(player->name);
            std::pair<float, int> ret = player->getCurrentScoresByRound(_currentRound);
            standardLabels[i]->setString(CompetitionResult::getStandardScoreString(ret.first));
            competitionLabels[i]->setString(std::to_string(ret.second));
        }
    }

    button->setUserData(reinterpret_cast<void *>(idx));

    return cell;
}

void CompetitionTableScene::onRecordButton(cocos2d::Ref *sender) {
    ui::Button *button = (ui::Button *)sender;
    size_t table = reinterpret_cast<size_t>(button->getUserData());
    const CompetitionTable &currentTable = _competitionTables->at(table);

    DrawNode *drawNode = DrawNode::create();

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 列宽
    const float colWidth[5] = {
        visibleSize.width * 0.1f,
        visibleSize.width * 0.2f,
        visibleSize.width * 0.15f,
        visibleSize.width * 0.15f,
        visibleSize.width * 0.15f,
    };

    // 中心位置
    const float posX[5] = {
        colWidth[0] * 0.5f,
        posX[0] + colWidth[0] * 0.5f + colWidth[1] * 0.5f,
        posX[1] + colWidth[1] * 0.5f + colWidth[2] * 0.5f,
        posX[2] + colWidth[2] * 0.5f + colWidth[3] * 0.5f,
        posX[3] + colWidth[3] * 0.5f + colWidth[4] * 0.5f
    };

    const float width = visibleSize.width * 0.75f;
    const float height = 100;
    drawNode->setContentSize(Size(width, height));

    drawNode->drawRect(Vec2(0, 0), Vec2(width, height), Color4F::BLACK);
    for (int i = 0; i < 4; ++i) {
        drawNode->drawLine(Vec2(0, i * 20 + 20), Vec2(width, i * 20 + 20), Color4F::BLACK);
    }


    for (int i = 0; i < 4; ++i) {
        const float x = posX[i] + colWidth[i] * 0.5f;
        drawNode->drawLine(Vec2(x, 0), Vec2(x, 100), Color4F::BLACK);
    }

    Label *labels[4][3] = { { nullptr } };

    const char *titleTexts[] = { "编号", "选手姓名", "顺位", "标准分", "比赛分" };
    for (int i = 0; i < 5; ++i) {
        Label *label = Label::createWithSystemFont(titleTexts[i], "Arail", 12);
        label->setColor(Color3B::BLACK);
        drawNode->addChild(label);
        label->setPosition(Vec2(posX[i], 90.0f));
        Common::scaleLabelToFitWidth(label, colWidth[i]);
    }

    std::vector<CompetitionPlayer> &players = _competitionData->players;

    for (int i = 0; i < 4; ++i) {
        CompetitionPlayer *player = &players[currentTable.player_indices[i]];

        Label *label = Label::createWithSystemFont(std::to_string(player->serial), "Arail", 12);
        label->setColor(Color3B::BLACK);
        drawNode->addChild(label);
        label->setPosition(Vec2(posX[0], 70.0f - 20.0f * i));
        Common::scaleLabelToFitWidth(label, colWidth[0]);

        label = Label::createWithSystemFont(player->name, "Arail", 12);
        label->setColor(Color3B::BLACK);
        drawNode->addChild(label);
        label->setPosition(Vec2(posX[1], 70.0f - 20.0f * i));
        Common::scaleLabelToFitWidth(label, colWidth[1]);

        for (int k = 0; k < 3; ++k) {
            label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            drawNode->addChild(label);
            label->setPosition(Vec2(posX[2 + k], 70.0f - 20.0f * i));
            labels[i][k] = label;
        }

        CompetitionResult *result = &player->competition_results[_currentRound];
        std::function<void ()> callback = [result, labels, i]() {
            std::string text[3] = {
                std::to_string(result->rank),
                CompetitionResult::getStandardScoreString(result->standard_score),
                std::to_string(result->competition_score)
            };
            for (int k = 0; k < 3; ++k) {
                labels[i][k]->setString(text[k]);
            }
        };
        callback();

        ui::Button *button = ui::Button::create();
        button->setScale9Enabled(true);
        button->setPosition(Vec2(posX[3], 70.0f - 20.0f * i));
        button->setContentSize(Size(colWidth[2] + colWidth[3]+ colWidth[4], 20.0f));
        drawNode->addChild(button);
        button->setUserData(label);
        button->addClickEventListener([this, player, result, callback](Ref *) {
            showCompetitionResultInputAlert(Common::format<64>("选手编号%u，姓名「%s」", player->serial, player->name.c_str()), result, callback);
        });
    }

    AlertView::showWithNode(Common::format<128>("第%lu桌成绩", (unsigned long)table + 1), drawNode, [this, table, labels]() {
        std::vector<CompetitionPlayer> &players = _competitionData->players;
        CompetitionTable &currentTable = _competitionTables->at(table);
        for (int i = 0; i < 4; ++i) {
            const std::string &rank = labels[i][0]->getString();
            const std::string &ss = labels[i][1]->getString();
            const std::string &cs = labels[i][2]->getString();

            CompetitionResult &result = players[currentTable.player_indices[i]].competition_results[_currentRound];
            result.rank = atoi(rank.c_str());
            result.standard_score = atof(ss.c_str());
            result.competition_score = atoi(cs.c_str());

            _tableView->updateCellAtIndex(table);
        }
    }, nullptr);
}

void CompetitionTableScene::showCompetitionResultInputAlert(const std::string &title, CompetitionResult *result, const std::function<void ()> &callback) {
    Node *rootNode = Node::create();
    rootNode->setContentSize(Size(115, 90));

    Label *label = Label::createWithSystemFont("顺位", "Arial", 12);
    label->setColor(Color3B::BLACK);
    rootNode->addChild(label);
    label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    label->setPosition(Vec2(5, 75));

    char buf[32];
    snprintf(buf, sizeof(buf), "%u", result->rank);

    ui::EditBox *editBoxes[3];

    ui::EditBox *editBox = ui::EditBox::create(Size(50.0f, 20.0f), ui::Scale9Sprite::create("source_material/btn_square_normal.png"));
    editBox->setInputFlag(ui::EditBox::InputFlag::SENSITIVE);
    editBox->setInputMode(ui::EditBox::InputMode::NUMERIC);
    editBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
    editBox->setFontColor(Color4B::BLACK);
    editBox->setFontSize(12);
    editBox->setText(buf);
    rootNode->addChild(editBox);
    editBox->setPosition(Vec2(85, 75));
    editBoxes[0] = editBox;

    label = Label::createWithSystemFont("标准分", "Arial", 12);
    label->setColor(Color3B::BLACK);
    rootNode->addChild(label);
    label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    label->setPosition(Vec2(5, 45));

    editBox = ui::EditBox::create(Size(50.0f, 20.0f), ui::Scale9Sprite::create("source_material/btn_square_normal.png"));
    editBox->setInputFlag(ui::EditBox::InputFlag::SENSITIVE);
    editBox->setInputMode(ui::EditBox::InputMode::DECIMAL);
    editBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
    editBox->setFontColor(Color4B::BLACK);
    editBox->setFontSize(12);
    editBox->setText(CompetitionResult::getStandardScoreString(result->standard_score).c_str());
    rootNode->addChild(editBox);
    editBox->setPosition(Vec2(85, 45));
    editBoxes[1] = editBox;

    label = Label::createWithSystemFont("比赛分", "Arial", 12);
    label->setColor(Color3B::BLACK);
    rootNode->addChild(label);
    label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    label->setPosition(Vec2(5, 15));

    snprintf(buf, sizeof(buf), "%d", result->competition_score);

    editBox = ui::EditBox::create(Size(50.0f, 20.0f), ui::Scale9Sprite::create("source_material/btn_square_normal.png"));
    editBox->setInputFlag(ui::EditBox::InputFlag::SENSITIVE);
    editBox->setInputMode(ui::EditBox::InputMode::NUMERIC);
    editBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
    editBox->setFontColor(Color4B::BLACK);
    editBox->setFontSize(12);
    editBox->setText(buf);
    rootNode->addChild(editBox);
    editBox->setPosition(Vec2(85, 15));
    editBoxes[2] = editBox;

    AlertView::showWithNode(title, rootNode, [this, editBoxes, title, result, callback]() {
        unsigned rank = 0;
        float standardScore = 0;
        int competitionScore = 0;

        const char *text = editBoxes[0]->getText();
        if (*text != '\0') {
            rank = atoi(text);
        }

        text = editBoxes[1]->getText();
        if (*text != '\0') {
            standardScore = (float)atof(text);
        }

        text = editBoxes[2]->getText();
        if (*text != '\0') {
            competitionScore = atoi(text);
        }

        if (rank < 1 || rank > 4) {
            AlertView::showWithMessage("登记成绩", "顺位只能是1到4", 12,
                std::bind(&CompetitionTableScene::showCompetitionResultInputAlert, this, title, result, std::ref(callback)), nullptr);
            return;
        }

        if (standardScore < 0) {
            AlertView::showWithMessage("登记成绩", "标准分必须大于0", 12,
                std::bind(&CompetitionTableScene::showCompetitionResultInputAlert, this, title, result, std::ref(callback)), nullptr);
            return;
        }

        result->rank = rank;
        result->standard_score = standardScore;
        result->competition_score = competitionScore;
        callback();
    }, nullptr);
}

void CompetitionTableScene::rankBySerial() {
    std::vector<CompetitionPlayer> &players = _competitionData->players;
    const size_t cnt = players.size();
    _competitionTables->resize(cnt / 4);
    for (size_t i = 0; i < cnt; ) {
        CompetitionTable &table = _competitionTables->at(i / 4);
        table.player_indices[0] = i++;
        table.player_indices[1] = i++;
        table.player_indices[2] = i++;
        table.player_indices[3] = i++;
    }
}

void CompetitionTableScene::rankBySerialSnake() {
    std::vector<CompetitionPlayer> &players = _competitionData->players;
    const size_t cnt = players.size();
    _competitionTables->resize(cnt / 4);

    size_t east = 0;
    size_t south = cnt / 2 - 1;
    size_t west = south + 1;
    size_t north = cnt - 1;
    for (size_t i = 0; i < cnt; i += 4) {
        CompetitionTable &table = _competitionTables->at(i / 4);
        table.player_indices[0] = east++;
        table.player_indices[1] = south--;
        table.player_indices[2] = west++;
        table.player_indices[3] = north--;
    }
}

void CompetitionTableScene::rankByRandom() {
    std::vector<CompetitionPlayer> &players = _competitionData->players;
    const size_t cnt = players.size();
    _competitionTables->resize(cnt / 4);

    std::vector<CompetitionPlayer *> temp;
    temp.reserve(players.size());
    std::transform(players.begin(), players.end(), std::back_inserter(temp), [](CompetitionPlayer &p) { return &p; });
    std::random_shuffle(temp.begin(), temp.end());

    for (size_t i = 0; i < cnt; ) {
        CompetitionTable &table = _competitionTables->at(i / 4);
        table.player_indices[0] = temp[i++] - &players[0];
        table.player_indices[1] = temp[i++] - &players[0];
        table.player_indices[2] = temp[i++] - &players[0];
        table.player_indices[3] = temp[i++] - &players[0];
    }
}

struct ScoresSortInfo {
    CompetitionPlayer *player;
    float standard_score;
    int competition_score;
};

static void caculateInfo(std::vector<CompetitionPlayer> &players, unsigned round, std::vector<ScoresSortInfo> &temp) {
    temp.reserve(players.size());
    std::transform(players.begin(), players.end(), std::back_inserter(temp), [round](CompetitionPlayer &p) {
        ScoresSortInfo ret;
        ret.player = &p;
        auto s = p.getTotalScoresByRound(round + 1);
        ret.standard_score = s.first;
        ret.competition_score = s.second;
        return ret;
    });
}

static void sortByInfo(std::vector<ScoresSortInfo> &temp, std::vector<ScoresSortInfo *> &ptemp) {
    ptemp.reserve(temp.size());
    std::transform(temp.begin(), temp.end(), std::back_inserter(ptemp), [](ScoresSortInfo &ssi) { return &ssi; });

    std::sort(ptemp.begin(), ptemp.end(), [](const ScoresSortInfo *a, const ScoresSortInfo *b) {
        if (a->standard_score > b->standard_score) return true;
        if (a->standard_score == b->standard_score && a->competition_score > b->competition_score) return true;
        return false;
    });
}

void CompetitionTableScene::rankByScores() {
    std::vector<CompetitionPlayer> &players = _competitionData->players;
    const size_t cnt = players.size();
    _competitionTables->resize(cnt / 4);

    std::vector<ScoresSortInfo> temp;
    caculateInfo(players, _currentRound, temp);

    std::vector<ScoresSortInfo *> ptemp;
    sortByInfo(temp, ptemp);

    // 排座位
    for (size_t i = 0; i < cnt; ) {
        CompetitionTable &table = _competitionTables->at(i / 4);
        table.player_indices[0] = ptemp[i++]->player - &players[0];
        table.player_indices[1] = ptemp[i++]->player - &players[0];
        table.player_indices[2] = ptemp[i++]->player - &players[0];
        table.player_indices[3] = ptemp[i++]->player - &players[0];
    }
}

void CompetitionTableScene::rankByScoresSnake() {
    std::vector<CompetitionPlayer> &players = _competitionData->players;
    const size_t cnt = players.size();
    _competitionTables->resize(cnt / 4);

    std::vector<ScoresSortInfo> temp;
    caculateInfo(players, _currentRound, temp);

    std::vector<ScoresSortInfo *> ptemp;
    sortByInfo(temp, ptemp);

    size_t east = 0;
    size_t south = cnt / 2 - 1;
    size_t west = south + 1;
    size_t north = cnt - 1;
    for (size_t i = 0; i < cnt; i += 4) {
        CompetitionTable &table = _competitionTables->at(i / 4);
        table.player_indices[0] = ptemp[east++]->player - &players[0];
        table.player_indices[1] = ptemp[south--]->player - &players[0];
        table.player_indices[2] = ptemp[west++]->player - &players[0];
        table.player_indices[3] = ptemp[north--]->player - &players[0];
    }
}

void CompetitionTableScene::onRankButton(cocos2d::Ref *sender) {
    Node *rootNode = Node::create();
    rootNode->setContentSize(Size(80, 120));

    ui::RadioButtonGroup *radioGroup = ui::RadioButtonGroup::create();
    rootNode->addChild(radioGroup);

    static const char *titles[] = { "随机", "编号蛇形", "排名蛇形", "高高碰", "自定义" };

    for (int i = 0; i < 5; ++i) {
        ui::RadioButton *radioButton = ui::RadioButton::create("source_material/btn_square_normal.png", "source_material/btn_square_highlighted.png");
        rootNode->addChild(radioButton);
        radioButton->setZoomScale(0.0f);
        radioButton->ignoreContentAdaptWithSize(false);
        radioButton->setContentSize(Size(20.0f, 20.0f));
        radioButton->setPosition(Vec2(10.0f, 110.0f - i * 25.0f));
        radioGroup->addRadioButton(radioButton);

        Label *label = Label::createWithSystemFont(titles[i], "Arial", 12);
        label->setColor(Color3B::BLACK);
        rootNode->addChild(label);
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(Vec2(25.0f, 110.0f - i * 25.0f));
    }

    AlertView::showWithNode("排列座位", rootNode, [this, radioGroup]() {
        switch (radioGroup->getSelectedButtonIndex()) {
        case 0: rankByRandom(); _tableView->reloadData(); break;
        case 1: rankBySerialSnake(); _tableView->reloadData(); break;
        case 2: rankByScoresSnake(); _tableView->reloadData(); break;
        case 3: rankByScores(); _tableView->reloadData(); break;
        case 4: break;
        default: break;
        }
    }, nullptr);
}
