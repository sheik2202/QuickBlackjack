#include <wx/wx.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

struct Card {
    std::string suit;  // "Heart", "Diamond", "Club", "Spade" // I will add this back into Face() return later
    int value;         // 1-13

    int BlackjackValue() const {
        if (value >= 10) return 10;  // face cards
        return value;                // Ace handled separately
    }

    std::string Face() const {
        std::string rank;
        switch (value) {
        case 1:  rank = "A"; break;
        case 11: rank = "J"; break;
        case 12: rank = "Q"; break;
        case 13: rank = "K"; break;
        default: rank = std::to_string(value); break;
        }
        std::string r = rank.size() == 1 ? rank + " " : rank;
        return "------\n|" + r + "  |\n|    |\n|  " + r + "|\n------";
    }
};

class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "Blackjack", wxDefaultPosition, wxSize(1200, 800))
    {
        panel = new wxPanel(this);
        wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* playerRow = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* dealerRow = new wxBoxSizer(wxHORIZONTAL);
        SetIcon(wxICON(aaaaBlackjack));

        // Value label
        valText = new wxStaticText(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

        // faces side by side
        // player faces
        playerCards = new wxStaticText(panel, wxID_ANY, "");
        // dealer faces
        dealerCards = new wxStaticText(panel, wxID_ANY, "");

        wxStaticText* playerLabel = new wxStaticText(panel, wxID_ANY, "Player:");
        wxStaticText* dealerLabel = new wxStaticText(panel, wxID_ANY, "Dealer:");
        playerRow->Add(playerLabel, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 10);
        playerRow->Add(playerCards, 1, wxEXPAND);
        dealerRow->Add(dealerLabel, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 10);
        dealerRow->Add(dealerCards, 1, wxEXPAND);

        // Reset button
        wxButton* resetBtn = new wxButton(panel, wxID_ANY, "Reset");
        resetBtn->Bind(wxEVT_BUTTON, &MainFrame::OnReset, this);
        // Hit button
        wxButton* hitBtn = new wxButton(panel, wxID_ANY, "Hit");
        hitBtn->Bind(wxEVT_BUTTON, &MainFrame::OnHit, this);
        // Stand button
        wxButton* standBtn = new wxButton(panel, wxID_ANY, "Stand");
        standBtn->Bind(wxEVT_BUTTON, &MainFrame::OnStand, this);

        wxBoxSizer* btnRow = new wxBoxSizer(wxHORIZONTAL); //
        btnRow->Add(resetBtn, 1, wxEXPAND); //
        btnRow->Add(hitBtn, 1, wxEXPAND); //
        btnRow->Add(standBtn, 1, wxEXPAND); //

        sizerV->Add(valText, 0, wxALIGN_CENTRE | wxTOP, 0);
        sizerV->Add(playerRow, 1, wxEXPAND | wxALL, 10);
        sizerV->Add(dealerRow, 1, wxEXPAND | wxALL, 10);
        sizerV->Add(btnRow, 1, wxEXPAND | wxALL, 30); //

        panel->SetSizer(sizerV);
        Bind(wxEVT_SIZE, &MainFrame::OnSize, this);

        BuildDeck();
        Deal();
        Show();
        CallAfter(&MainFrame::ReFont);
    }
private:
    wxPanel* panel = nullptr;
    bool gameOver = false;
    std::vector<Card> deck;
    std::vector<Card> playerHand;
    std::vector<Card> dealerHand;
    wxStaticText* valText = nullptr;
    wxStaticText* playerCards = nullptr;
    wxStaticText* dealerCards = nullptr;

    wxString BuildScreen(const std::vector<Card>& hand) {
        std::vector<std::vector<wxString>> cardRows;
        for (const auto& c : hand) {
            wxString face = c.Face();
            std::vector<wxString> rows;
            wxString row;
            for (char ch : face.ToStdString()) {
                if (ch == '\n') { rows.push_back(row); row = ""; }
                else row += ch;
            }
            rows.push_back(row);
            cardRows.push_back(rows);
        }
        // Combine row by row across all cards
        wxString result;
        for (int row = 0; row < 5; row++) {
            for (const auto& card : cardRows)
                result += card[row] + "  ";
            result += "\n";
        }
        return result;
    }

    void BuildDeck() {
        deck.clear();
        const std::string suits[] = { "H", "D", "C", "S" };
        for (const auto& suit : suits)
            for (int v = 1; v <= 13; v++)
                deck.push_back(Card{ suit, v });
        for (int i = (int)deck.size() - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(deck[i], deck[j]);
        }
    }
    Card DrawCard() {
        if (deck.empty()) BuildDeck(); // reshuffle if exhausted
        Card c = deck.back();
        deck.pop_back();
        return c;
    }
    int HandTotal(const std::vector<Card>& hand) {
        int total = 0, aces = 0;
        for (const auto& c : hand) {
            total += c.BlackjackValue();
            if (c.value == 1) aces++;
        }
        while (aces > 0 && total + 10 <= 21) {
            total += 10;
            aces--;
        }
        return total;
    }
    void Deal() {
        playerHand.clear();
        dealerHand.clear();
        gameOver = false;
        playerHand.push_back(DrawCard());
        playerHand.push_back(DrawCard());
        dealerHand.push_back(DrawCard());
        dealerHand.push_back(DrawCard());
        UpdateScreen();
    }
    void UpdateScreen() {
        int playerTotal = HandTotal(playerHand);
        int dealerTotal = HandTotal(dealerHand);

        valText->SetLabel(wxString::Format("Player: %d     Dealer: %d", playerTotal, dealerTotal));

        playerCards->SetLabel(BuildScreen(playerHand));
        dealerCards->SetLabel(BuildScreen(dealerHand));
        ReFont();
        panel->Layout();
    }
    void ReFont() {
        wxSize size = GetClientSize();
        int numSize = std::max(10, std::min(size.y / 8, 30));
        valText->SetFont(wxFont(numSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        valText->InvalidateBestSize();
        int cardCount = std::max((int)playerHand.size(), (int)dealerHand.size());
        cardCount = std::max(cardCount, 2);
        int faceSize = std::max(6, std::min(size.x / (cardCount * 10), std::min(size.y / 20, 30)));
        wxFont faceFont(faceSize, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        playerCards->SetFont(faceFont);
        playerCards->InvalidateBestSize();
        dealerCards->SetFont(faceFont);
        dealerCards->InvalidateBestSize();
        panel->Layout();
    }
    void Hit() {
        if (gameOver) return;
        playerHand.push_back(DrawCard());
        if (HandTotal(playerHand) > 21) {
            gameOver = true;
            UpdateScreen();
            valText->SetLabel(wxString::Format("Player: %d  --  Bust!", HandTotal(playerHand)));
            ReFont();
            return;
        }
        UpdateScreen();
    }
    void Stand() {
        if (gameOver) return;
        gameOver = true;
        // Dealer hits until 17
        while (HandTotal(dealerHand) < 17) dealerHand.push_back(DrawCard());
        int p = HandTotal(playerHand);
        int d = HandTotal(dealerHand);
        wxString result;
        if (d > 21 || p > d) result = "You win!";
        else if (p < d) result = "Dealer wins!";
        else result = "Push!";
        UpdateScreen();
        valText->SetLabel(wxString::Format("Player: %d  Dealer: %d  --  ", p, d) + result);
        ReFont();
    }
    void OnSize(wxSizeEvent& event) { event.Skip(); ReFont(); panel->Layout(); }
    void OnReset(wxCommandEvent&) { Deal(); }
    void OnHit(wxCommandEvent&) { Hit(); }
    void OnStand(wxCommandEvent&) { Stand(); }
};
class App : public wxApp {
public:
    bool OnInit() {
        srand(time(0));
        new MainFrame();
        return true;
    }
};

wxIMPLEMENT_APP(App);
