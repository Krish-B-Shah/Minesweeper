#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
using namespace std;

//ALL THE FUNCTIONS:
// Utility: Get random number in range [0, max)
int getRandom(int max) {
    return rand() % max;
}

// SHOWS THE THE IMAGE FOR CLOCK AND THE FLAG COUNT
void displayNumber(sf::RenderWindow &window, int number, float x, float y, sf::Texture &digitsTexture) {
   bool isNegative = number < 0;
   if (isNegative) {
       number = -number;
        //NEGATIVE SIGN LOGIC:
       sf::Sprite negativeSign(digitsTexture);
       negativeSign.setTextureRect(sf::IntRect(10 * 21, 0, 21, 32));
       negativeSign.setPosition(x, y);
       window.draw(negativeSign);
       x += 21;
   }
   std::string numStr = std::to_string(number);

   while (numStr.length() < 3) {
       numStr = "0" + numStr;
   }
    for (char c : numStr) {
        int digit = c - '0';
        sf::Sprite digitSprite(digitsTexture);
        digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        digitSprite.setPosition(x, y);
        window.draw(digitSprite);
        x += 21;
    }

}

// READ THE CONFIG FILE:
void readConfig(int &columns, int &rows, int& mines) {
   ifstream configFile("files/config.cfg");
   string line;
   if (configFile.is_open()) {
       getline(configFile, line);
       columns = stoi(line);
       getline(configFile, line);
       rows = stoi(line);
       getline(configFile, line);
       mines = stoi(line);
       configFile.close();
   }
}

// SETTING THE TEXT POSITION:
void setText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(x, y);
}


// CHECK THE ADJACENT TILES:
int checkAdjacentMines(std::vector<std::vector<bool>> &mineGrid, int rowCount, int columnCount, int row, int column) {
    int count = 0;

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            int newRow = row + i;
            int newCol = column + j;
            if (newRow >= 0 && newRow < rowCount && newCol >= 0 && newCol < columnCount) {
                if (mineGrid[newRow][newCol]) count++;
            }
        }
    }

    return count;
}

// REVEAL ALL MINES
void revealAllMines(vector<vector<bool>>& mineGrid, vector<vector<bool>>& revealed, vector<vector<bool>>& flagged, int rowCount, int columnCount) {
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
            if (mineGrid[i][j]) {
                revealed[i][j] = true;
            }
        }
    }
}

// RECURSIVE FUNCTION TO REVEAL EMPTY TILES
void revealEmptyTiles(int row, int col,
                      vector<vector<bool>>& revealed,
                      vector<vector<int>>& adjacentMines,
                      vector<vector<bool>>& mineGrid,
                      vector<vector<bool>>& flagged,
                      int totalRows, int totalCols) {
    if (row < 0 || row >= totalRows || col < 0 || col >= totalCols) return;
    if (revealed[row][col] || flagged[row][col] || mineGrid[row][col]) return;
    int minesNearby = checkAdjacentMines(mineGrid, totalRows, totalCols, row, col);
    revealed[row][col] = true;
    adjacentMines[row][col] = minesNearby;
    if (minesNearby == 0) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0)
                    revealEmptyTiles(row + dx, col + dy, revealed, adjacentMines, mineGrid, flagged, totalRows, totalCols);
            }
        }
    }
}

//ALL THE WINDOWS:
void leaderBoardWindow(sf::Font& font, bool& leaderboardOpen, std::string currentPlayerName) {
    int rowCount, columnCount, minesCount;
    readConfig(columnCount, rowCount, minesCount);

    sf::RenderWindow leaderboardWindow(sf::VideoMode(columnCount * 32, rowCount * 32 + 100), "Leaderboard", sf::Style::Close);
    sf::RectangleShape rectangle(sf::Vector2f(columnCount * 32, rowCount * 32 + 100));
    rectangle.setFillColor(sf::Color::Blue);

    std::vector<std::pair<std::string, int>> scores;
    std::ifstream inFile("files/leaderboard.txt");
    std::string name;
    int score;

    while (inFile >> name >> score) {
        if (!name.empty() && name.back() == '*') {
            name.pop_back();
        }
        scores.emplace_back(name, score);
    }
    inFile.close();

    for (size_t i = 1; i < scores.size(); ++i) {
        auto key = scores[i];
        int j = i - 1;
        while (j >= 0 && scores[j].second > key.second) {
            scores[j + 1] = scores[j];
            j--;
        }
        scores[j + 1] = key;
    }

    if (scores.size() > 5) {
        scores.resize(5);
    }

    while (leaderboardWindow.isOpen()) {
        sf::Event event;
        while (leaderboardWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                leaderboardWindow.close();
                leaderboardOpen = false;
            }
        }

        leaderboardWindow.clear();
        leaderboardWindow.draw(rectangle);

        sf::Text title;
        title.setFont(font);
        title.setString("LEADERBOARD");
        title.setCharacterSize(24);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Underlined | sf::Text::Bold);
        title.setPosition(columnCount * 16 - title.getLocalBounds().width / 2, 10);
        leaderboardWindow.draw(title);

        for (size_t i = 0; i < scores.size(); ++i) {
            sf::Text entry;
            entry.setFont(font);
            entry.setCharacterSize(25);
            entry.setFillColor(sf::Color::White);

            int minutes = scores[i].second / 60;
            int seconds = scores[i].second % 60;
            std::string timeStr =
                (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
                (seconds < 10 ? "0" : "") + std::to_string(seconds);

            std::ostringstream oss;
            if (scores[i].first == currentPlayerName) {
                oss << setw(3) << std::right << i + 1 << ".   "
                    << setw(5) << std::left << timeStr << "   "
                    << scores[i].first << "*";
            } else {
                oss << setw(3) << std::right << i + 1 << ".   "
                    << setw(5) << std::left << timeStr << "   "
                    << scores[i].first;
            }

            entry.setString(oss.str());

            // Center align the whole string
            sf::FloatRect bounds = entry.getLocalBounds();
            entry.setOrigin(bounds.left + bounds.width / 2.f, 0);
            entry.setPosition((columnCount * 32) / 2.0f, 60 + static_cast<float>(i * 100));
            leaderboardWindow.draw(entry);
        }

        leaderboardWindow.display();
    }
}

// GAME WINDOW FUNCTION
void gameWindow(sf::Font& font, const std::string& playerName) {
    int columnCount, rowCount, mineCount;
    readConfig(columnCount, rowCount, mineCount);

    std::vector<std::vector<bool>> mineGrid(rowCount, std::vector<bool>(columnCount, false));
                 std::vector<std::vector<bool>> revealed(rowCount, std::vector<bool>(columnCount, false));
    std::vector<std::vector<bool>> flagged(rowCount, std::vector<bool>(columnCount, false));
          std::vector<std::vector<int>> adjacentMines(rowCount, std::vector<int>(columnCount, 0));

    srand(static_cast<unsigned>(time(nullptr)));
    int placedMines = 0;
    while (placedMines < mineCount) {
        int randRow = getRandom(rowCount);
        int randCol = getRandom(columnCount);
        if (!mineGrid[randRow][randCol]) {
            mineGrid[randRow][randCol] = true;
            placedMines++;
        }
    }

                for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
                       adjacentMines[i][j] = checkAdjacentMines(mineGrid, rowCount, columnCount, i, j);
        }
                 }

    sf::RenderWindow gameWindow(sf::VideoMode(columnCount * 32, rowCount * 32 + 100), "Minesweeper Game");

                     sf::Texture tileTexture;
    sf::Texture happyTexture;
    sf::Texture victoryTexture;
    sf::Texture defeatTexture;
    sf::Texture minesButtonTexture;
    sf::Texture pauseTexture;
    sf::Texture playTexture;
    sf::Texture leaderTexture;
    sf::Texture digitsTexture;
    sf::Texture debugButtonTexture;
    sf::Texture flagTexture;
             sf::Texture revealedTexture;
    sf::Texture number1Texture;
    sf::Texture number2Texture;
    sf::Texture number3Texture;
    sf::Texture number4Texture;
    sf::Texture number5Texture;
    sf::Texture number6Texture;
    sf::Texture number7Texture;
    sf::Texture number8Texture;

 if (!tileTexture.loadFromFile("files/images/tile_hidden.png")) {
    cout << "Error loading tile_hidden.png!" << endl;
    return;
}
if (!minesButtonTexture.loadFromFile("files/images/mine.png")) {
    cout << "Error loading mine.png!" << endl;
    return;
}
if (!debugButtonTexture.loadFromFile("files/images/debug.png")) {
    cout << "Error loading debug.png!" << endl;
    return;
}
if (!happyTexture.loadFromFile("files/images/face_happy.png")) {
    cout << "Error loading face_happy.png!" << endl;
    return;
}
if (!pauseTexture.loadFromFile("files/images/pause.png")) {
    cout << "Error loading pause.png!" << endl;
    return;
}
if (!playTexture.loadFromFile("files/images/play.png")) {
    cout << "Error loading play.png!" << endl;
    return;
}
if (!leaderTexture.loadFromFile("files/images/leaderboard.png")) {
    cout << "Error loading leaderboard.png!" << endl;
    return;
}
if (!digitsTexture.loadFromFile("files/images/digits.png")) {
    cout << "Error loading digits.png!" << endl;
    return;
}
if (!flagTexture.loadFromFile("files/images/flag.png")) {
    cout << "Error loading flag.png!" << endl;
    return;
}
if (!number1Texture.loadFromFile("files/images/number_1.png")) {
    cout << "Error loading number_1.png!" << endl;
    return;
}
if (!number2Texture.loadFromFile("files/images/number_2.png")) {
    cout << "Error loading number_2.png!" << endl;
    return;
}
if (!number3Texture.loadFromFile("files/images/number_3.png")) {
    cout << "Error loading number_3.png!" << endl;
    return;
}
if (!number4Texture.loadFromFile("files/images/number_4.png")) {
    cout << "Error loading number_4.png!" << endl;
    return;
}
if (!number5Texture.loadFromFile("files/images/number_5.png")) {
    cout << "Error loading number_5.png!" << endl;
    return;
}
if (!number6Texture.loadFromFile("files/images/number_6.png")) {
    cout << "Error loading number_6.png!" << endl;
    return;
}
if (!number7Texture.loadFromFile("files/images/number_7.png")) {
    cout << "Error loading number_7.png!" << endl;
    return;
}
if (!number8Texture.loadFromFile("files/images/number_8.png")) {
    cout << "Error loading number_8.png!" << endl;
    return;
}
if (!revealedTexture.loadFromFile("files/images/tile_revealed.png")) {
    cout << "Error loading tile_revealed.png!" << endl;
    return;
}
if (!victoryTexture.loadFromFile("files/images/face_win.png")) {
    cout << "Error loading face_win.png!" << endl;
    return;
}
if (!defeatTexture.loadFromFile("files/images/face_lose.png")) {
    cout << "Error loading face_lose.png!" << endl;
    return;
}


    // Create sprites with textures
    sf::Sprite tileSprite(tileTexture);
    sf::Sprite happyFace(happyTexture);
    sf::Sprite mineReveal(minesButtonTexture);
    sf::Sprite pauseFace(pauseTexture);
    sf::Sprite leaderboard(leaderTexture);
    sf::Sprite debugButton(debugButtonTexture);
    sf::Sprite flag(flagTexture);
    sf::Sprite number1(number1Texture);
    sf::Sprite number2(number2Texture);
    sf::Sprite number3(number3Texture);
    sf::Sprite number4(number4Texture);
    sf::Sprite number5(number5Texture);
    sf::Sprite number6(number6Texture);
    sf::Sprite number7(number7Texture);
    sf::Sprite number8(number8Texture);
    sf::Sprite reveal(revealedTexture);

    int remainingMines = mineCount;
    bool gamePaused = false;
    bool debug = false;
    bool gameOver = false;
    bool leaderboardOpen = false;
    bool showAllRevealed = false;

    std::vector<std::vector<bool>> savedRevealed;

    int timeElapsed = 0;
    float pauseTime = 0;

    sf::Clock gameClock;

    happyFace.setPosition((columnCount * 32) / 2 - 16, rowCount * 32 + 16);
    debugButton.setPosition((columnCount * 32) - 304, rowCount * 32 + 16);
    pauseFace.setPosition((columnCount * 32) - 240, rowCount * 32 + 16);
    leaderboard.setPosition((columnCount * 32) - 176, rowCount * 32 + 16);

    while (gameWindow.isOpen()) {
        sf::Event event;
        while (gameWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                gameWindow.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(gameWindow);
                int col = mousePos.x / 32;
                int row = mousePos.y / 32;

                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (mousePos.x >= happyFace.getPosition().x &&
                        mousePos.x <= happyFace.getPosition().x + 64 &&
                        mousePos.y >= happyFace.getPosition().y &&
                        mousePos.y <= happyFace.getPosition().y + 64) {

                        // Reset game logic
                        for (int i = 0; i < rowCount; i++) {
                            for (int j = 0; j < columnCount; j++) {
                                mineGrid[i][j] = false;
                                revealed[i][j] = false;
                                flagged[i][j] = false;
                                adjacentMines[i][j] = 0;
                            }
                        }

                                        placedMines = 0;
                                                            while (placedMines < mineCount) {
                                                                int randRow = rand() % rowCount;
                                                                    int randCol = rand() % columnCount;

                                                                if (!mineGrid[randRow][randCol]) {
                                                                    mineGrid[randRow][randCol] = true;
                                                                    placedMines++;
                                                                }
                        }

                        for (int i = 0; i < rowCount; i++) {
                            for (int j = 0; j < columnCount; j++) {
                                adjacentMines[i][j] = checkAdjacentMines(mineGrid, rowCount, columnCount, i, j);
                            }
                        }

                                    remainingMines = mineCount;
                                    timeElapsed = 0;
                        pauseTime = 0;
                        gameClock.restart();
                        pauseFace.setTexture(pauseTexture);
                                        happyFace.setTexture(happyTexture);
                                        gamePaused = false;
                        gameOver = false;
                        debug = false;
                        showAllRevealed = false;
                    }
                    else if (mousePos.x >= pauseFace.getPosition().x &&
                             mousePos.x <= pauseFace.getPosition().x + 64 &&
                             mousePos.y >= pauseFace.getPosition().y &&
                             mousePos.y <= pauseFace.getPosition().y + 64) {

                        if (!gameOver && !leaderboardOpen) {
                            gamePaused = !gamePaused;
                            if (gamePaused) {
                                pauseTime = timeElapsed;
                                pauseFace.setTexture(playTexture);
                            } else {
                                pauseFace.setTexture(pauseTexture);
                                gameClock.restart();
                            }
                        }
                    }
                    else if (mousePos.x >= leaderboard.getPosition().x &&
                             mousePos.x <= leaderboard.getPosition().x + 64 &&
                             mousePos.y >= leaderboard.getPosition().y &&
                             mousePos.y <= leaderboard.getPosition().y + 64) {

                        if (!leaderboardOpen) {
                            savedRevealed = revealed;
                            leaderboardOpen = true;

                            showAllRevealed = true;

                            bool wasPaused = gamePaused;
                            gamePaused = true;

                            leaderBoardWindow(font, leaderboardOpen, playerName);

                            showAllRevealed = false;
                            revealed = savedRevealed;

                            // Restore pause state
                            gamePaused = wasPaused;
                        }
                    }
                    else if (mousePos.x >= debugButton.getPosition().x &&
                             mousePos.x <= debugButton.getPosition().x + 64 &&
                             mousePos.y >= debugButton.getPosition().y &&
                             mousePos.y <= debugButton.getPosition().y + 64) {

                        if (!gameOver && !leaderboardOpen) {
                            debug = !debug;
                        }
                    }
                    else if (!gamePaused && !gameOver && !leaderboardOpen &&
                             col >= 0 && col < columnCount && row >= 0 && row < rowCount &&
                             !flagged[row][col] && !revealed[row][col]) {
                        if (mineGrid[row][col]) {
                            revealAllMines(mineGrid, revealed, flagged, rowCount, columnCount);
                            gameOver = true;
                                    gamePaused = true;
                            pauseFace.setTexture(playTexture);
                                   happyFace.setTexture(defeatTexture);
                                  cout << "You clicked on a mine! Game Over!" << endl;
                        } else {
                            revealEmptyTiles(row, col, revealed, adjacentMines, mineGrid, flagged, rowCount, columnCount);
                        }
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right &&
                         !gamePaused && !gameOver && !leaderboardOpen &&
                         col >= 0 && col < columnCount && row >= 0 && row < rowCount &&
                         !revealed[row][col]) {
                                 flagged[row][col] = !flagged[row][col];
                    remainingMines += flagged[row][col] ? -1 : 1;
                }
            }
        }

        if (!gamePaused && !gameOver) {
            timeElapsed = static_cast<int>(gameClock.getElapsedTime().asSeconds()) + pauseTime;
        }

        bool hasWon = true;
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < columnCount; j++) {
                if (!mineGrid[i][j] && !revealed[i][j]) {
                    hasWon = false;
                    break;
                }
            }
            if (!hasWon) break;
        }

        if (hasWon && !gameOver) {
            timeElapsed = static_cast<int>(gameClock.getElapsedTime().asSeconds()) + pauseTime;
            gameOver = true;

            for (int i = 0; i < rowCount; i++) {
                for (int j = 0; j < columnCount; j++) {
                    if (mineGrid[i][j] && !flagged[i][j]) {
                        flagged[i][j] = true;
                        remainingMines--;
                    }
                }
            }

            happyFace.setTexture(victoryTexture);

            gamePaused = true;
            pauseFace.setTexture(playTexture);

            debug = false;

            std::vector<std::pair<std::string, int>> scores;
            std::ifstream inFile("files/leaderboard.txt");
            std::string name;
            int score;
            while (inFile >> name >> score) {
                if (!name.empty() && name.back() == '*') {
                    name.pop_back();
                }
                scores.emplace_back(name, score);
            }
            inFile.close();

            scores.emplace_back(playerName, timeElapsed);

            std::sort(scores.begin(), scores.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second < b.second;
            });

            if (scores.size() > 5) scores.resize(5);

            std::ofstream outFile("files/leaderboard.txt");
            for (const auto& s : scores) {
                outFile << s.first << " " << s.second << "\n";
            }
            outFile.close();
        }

        gameWindow.clear(sf::Color::White);

        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < columnCount; j++) {
                if (!revealed[i][j] && !showAllRevealed) {
                    tileSprite.setPosition(j * 32, i * 32);
                    gameWindow.draw(tileSprite);

                    if (flagged[i][j]) {
                        flag.setPosition(j * 32, i * 32);
                        gameWindow.draw(flag);
                    }

                    if (debug && mineGrid[i][j]) {
                        mineReveal.setPosition(j * 32, i * 32);
                        gameWindow.draw(mineReveal);
                    }
                } else {
                    // Draw revealed tile (always show in leaderboard mode)
                    reveal.setPosition(j * 32, i * 32);
                    gameWindow.draw(reveal);

                    // Draw mine if present and game is over, or in debug mode
                    if (mineGrid[i][j]) {
                        mineReveal.setPosition(j * 32, i * 32);
                        gameWindow.draw(mineReveal);
                    }
                    else if (!showAllRevealed) {
                        int mineCount = adjacentMines[i][j];

                        if (mineCount == 1) {
                            number1.setPosition(j * 32, i * 32);
                            gameWindow.draw(number1);
                        }
                        else if (mineCount == 2) {
                            number2.setPosition(j * 32, i * 32);
                            gameWindow.draw(number2);
                        }
                        else if (mineCount == 3) {
                            number3.setPosition(j * 32, i * 32);
                            gameWindow.draw(number3);
                        }
                        else if (mineCount == 4) {
                            number4.setPosition(j * 32, i * 32);
                            gameWindow.draw(number4);
                        }
                        else if (mineCount == 5) {
                            number5.setPosition(j * 32, i * 32);
                            gameWindow.draw(number5);
                        }
                        else if (mineCount == 6) {
                            number6.setPosition(j * 32, i * 32);
                            gameWindow.draw(number6);
                        }
                        else if (mineCount == 7) {
                            number7.setPosition(j * 32, i * 32);
                            gameWindow.draw(number7);
                        }
                        else if (mineCount == 8) {
                            number8.setPosition(j * 32, i * 32);
                            gameWindow.draw(number8);
                        }
                    }
                }
            }
        }

        gameWindow.draw(happyFace);
        gameWindow.draw(debugButton);
        gameWindow.draw(pauseFace);
        gameWindow.draw(leaderboard);

        displayNumber(gameWindow, remainingMines, 33, rowCount * 32 + 32, digitsTexture);

        int minutes = timeElapsed / 60;
        int seconds = timeElapsed % 60;

        float timerX = columnCount * 32 - 105; // right aligned like before
        float timerY = rowCount * 32 + 32;

        std::string a = "";
        if (minutes < 10) {
            std::ostringstream b;
            b << "0";
            b << minutes;
            a = b.str();
        } else {
            std::ostringstream c;
            c << minutes;
            a = c.str();
        }

        for (int d = 0; d < a.length(); d++) {
            char e = a.at(d);

            int f = 0;
            std::istringstream g(std::string(1, e));
            g >> f;

            std::vector<int> h;
            for (int i = 0; i < 1000; i++) {
                h.push_back(0);
            }

            sf::Sprite* j = new sf::Sprite(digitsTexture);
            sf::IntRect k(f * 21, 0, 21, 32);
            j->setTextureRect(k);
            j->setPosition(timerX, timerY);
            gameWindow.draw(*j);
            timerX += 21;

            delete j;
            h.clear();
        }

        sf::Sprite* l = new sf::Sprite(digitsTexture);
        l->setTextureRect(sf::IntRect(10 * 21, 0, 21, 32));
        l->setPosition(timerX, timerY);
        gameWindow.draw(*l);
        timerX += 21;
        delete l;

        std::string m = "";
        if (seconds < 10) {
            std::ostringstream n;
            n << "0";
            n << seconds;
            m = n.str();
        } else {
            std::ostringstream o;
            o << seconds;
            m = o.str();
        }

        int p = 0;
        while (p < m.length()) {
            char q = m[p];

            int r = 0;
            std::stringstream s;
            s << q;
            s >> r;

            for (int t = 0; t < 500; t++) {
                r += 0;
            }

            sf::Sprite* u = new sf::Sprite(digitsTexture);
            u->setTextureRect(sf::IntRect(r * 21, 0, 21, 32));
            u->setPosition(timerX, timerY);
            gameWindow.draw(*u);
            timerX += 21;

            delete u;
            p++;
        }

        gameWindow.display();

    }
}

int main() {
    int columnCount, rowCount, minesCount;
    readConfig(columnCount, rowCount, minesCount);

    int width = columnCount * 32;
    int height = rowCount * 32 + 100;

    sf::RenderWindow welcomeWindow(sf::VideoMode(width, height), "Minesweeper", sf::Style::Close);
    sf::RectangleShape rectangle(sf::Vector2f(width, height));
    rectangle.setFillColor(sf::Color::Blue);

    sf::Font font;
    if (!font.loadFromFile("files/font.ttf")) {
        std::cout << "Error loading fonts" << std::endl;
        return 1;
    }

    sf::Text title;
    title.setFont(font);
    title.setString("Welcome to Minesweeper!");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Underlined | sf::Text::Bold);
    setText(title, width / 2, height / 2 - 180);

    sf::Text prompt;
    prompt.setFont(font);
    prompt.setString("Enter your name: ");
    prompt.setCharacterSize(22);
    prompt.setFillColor(sf::Color::White);
    prompt.setStyle(sf::Text::Bold);
    setText(prompt, width / 2, height / 2 - 120);

    sf::Text yourname;
    std::string inputName;
    yourname.setFont(font);
    yourname.setCharacterSize(20);
    yourname.setFillColor(sf::Color::Yellow);
    yourname.setStyle(sf::Text::Bold);
    setText(yourname, width / 2, height / 2 - 90);

    // Difficulty selection
    std::string difficulties[3] = {"Easy", "Medium", "Hard"};
    int selectedDifficulty = 0;
    sf::Text difficultyText[3];
    for (int i = 0; i < 3; ++i) {
        difficultyText[i].setFont(font);
        difficultyText[i].setString(difficulties[i]);
        difficultyText[i].setCharacterSize(20);
        difficultyText[i].setFillColor(i == selectedDifficulty ? sf::Color::Green : sf::Color::White);
        setText(difficultyText[i], width / 2 - 80 + i * 80, height / 2);
    }

    // Help and Settings buttons
    sf::Text helpBtn, settingsBtn;
    helpBtn.setFont(font);
    helpBtn.setString("Help");
    helpBtn.setCharacterSize(18);
    helpBtn.setFillColor(sf::Color::Cyan);
    setText(helpBtn, width / 2 - 60, height / 2 + 60);
    settingsBtn.setFont(font);
    settingsBtn.setString("Settings");
    settingsBtn.setCharacterSize(18);
    settingsBtn.setFillColor(sf::Color::Cyan);
    setText(settingsBtn, width / 2 + 60, height / 2 + 60);

    sf::RectangleShape cursor_shape(sf::Vector2f(5, 20));
    cursor_shape.setFillColor(sf::Color::White);

    bool showHelp = false;
    bool showSettings = false;
    bool startGame = false;

    while (welcomeWindow.isOpen()) {
        sf::Event event;
        while (welcomeWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                welcomeWindow.close();
            }
            if (!showHelp && !showSettings) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter && !inputName.empty()) {
                    startGame = true;
                    welcomeWindow.close();
                }
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == '\b' && !inputName.empty()) {
                        inputName.pop_back();
                    } else if (event.text.unicode < 128 && isalpha(event.text.unicode) && inputName.size() < 10) {
                        inputName += static_cast<char>(event.text.unicode);
                    }
                    if (!inputName.empty()) {
                        inputName[0] = toupper(inputName[0]);
                        for (size_t i = 1; i < inputName.size(); ++i) {
                            inputName[i] = tolower(inputName[i]);
                        }
                    }
                    yourname.setString(inputName);
                    setText(yourname, width / 2, height / 2 - 90);
                }
                if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(welcomeWindow);
                    for (int i = 0; i < 3; ++i) {
                        sf::FloatRect bounds = difficultyText[i].getGlobalBounds();
                        if (bounds.contains(mousePos.x, mousePos.y)) {
                            selectedDifficulty = i;
                            for (int j = 0; j < 3; ++j)
                                difficultyText[j].setFillColor(j == selectedDifficulty ? sf::Color::Green : sf::Color::White);
                        }
                    }
                    if (helpBtn.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        showHelp = true;
                    }
                    if (settingsBtn.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        showSettings = true;
                    }
                }
            } else if (showHelp) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    showHelp = false;
                }
            } else if (showSettings) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    showSettings = false;
                }
            }
            float nameTextX = yourname.getPosition().x + yourname.getLocalBounds().width / 2.f + 2;
            float nameTextY = yourname.getPosition().y - yourname.getLocalBounds().height / 2.f;
            cursor_shape.setPosition(nameTextX, nameTextY);
        }
        welcomeWindow.clear();
        welcomeWindow.draw(rectangle);
        welcomeWindow.draw(title);
        welcomeWindow.draw(prompt);
        welcomeWindow.draw(yourname);
        welcomeWindow.draw(cursor_shape);
        for (int i = 0; i < 3; ++i) welcomeWindow.draw(difficultyText[i]);
        welcomeWindow.draw(helpBtn);
        welcomeWindow.draw(settingsBtn);
        if (showHelp) {
            sf::RectangleShape helpBg(sf::Vector2f(width - 100, height - 100));
            helpBg.setFillColor(sf::Color(0,0,0,200));
            helpBg.setPosition(50, 50);
            welcomeWindow.draw(helpBg);
            sf::Text helpText;
            helpText.setFont(font);
            helpText.setCharacterSize(18);
            helpText.setFillColor(sf::Color::White);
            helpText.setString("Minesweeper Instructions:\n\n- Left click to reveal a tile.\n- Right click to flag/unflag a tile.\n- Reveal all non-mine tiles to win.\n- Use the buttons for pause, debug, leaderboard.\n- Press ESC to close this window.");
            helpText.setPosition(70, 70);
            welcomeWindow.draw(helpText);
        }
        if (showSettings) {
            sf::RectangleShape settingsBg(sf::Vector2f(width - 100, height - 100));
            settingsBg.setFillColor(sf::Color(0,0,0,200));
            settingsBg.setPosition(50, 50);
            welcomeWindow.draw(settingsBg);
            sf::Text settingsText;
            settingsText.setFont(font);
            settingsText.setCharacterSize(18);
            settingsText.setFillColor(sf::Color::White);
            settingsText.setString("Settings (future):\n- Sound: On/Off\n- Theme: Light/Dark\n- Language: English\n- Press ESC to close this window.");
            settingsText.setPosition(70, 70);
            welcomeWindow.draw(settingsText);
        }
        welcomeWindow.display();
    }

    // Set config based on difficulty
    if (startGame) {
        int diffCols[3] = {9, 16, 24};
        int diffRows[3] = {9, 16, 24};
        int diffMines[3] = {10, 40, 99};
        std::ofstream configFile("files/config.cfg");
        configFile << diffCols[selectedDifficulty] << "\n" << diffRows[selectedDifficulty] << "\n" << diffMines[selectedDifficulty] << "\n";
        configFile.close();
        gameWindow(font, inputName);
    }

    return 0;
}