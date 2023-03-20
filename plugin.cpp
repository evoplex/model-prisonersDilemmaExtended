/**
 * Copyright (c) 2023 - Marcos Cardinot <marcos@cardinot.net>
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "plugin.h"

namespace evoplex {

bool PDGame::init()
{
    const auto read = [this](auto& var, const auto& strategy)
    {
        var = attr(strategy, -999999.).toDouble();
        if (var < -1000. || var > 1000.) {
            qDebug() << "Invalid payoff! " << strategy << var;
            return false;
        }
        return true;
    };
    return read(m_cc, "cc") && read(m_cd, "cd") && read(m_dc, "dc") && read(m_dd, "dd");
}

bool PDGame::algorithmStep()
{
    // 1. each agent accumulates the payoff obtained by playing
    //    the game with all its neighbours and itself
    for (Node node : nodes()) {
        const int sX = node.attr(STRATEGY).toInt();
        double score = playGame(sX, sX);
        for (const Node& neighbour : node.outEdges()) {
            score += playGame(sX, neighbour.attr(STRATEGY).toInt());
        }
        node.setAttr(SCORE, score);
    }

    std::vector<char> bestStrategies;
    bestStrategies.reserve(nodes().size());

    // 2. the best agent in the neighbourhood is selected to reproduce
    for (const Node& node : nodes()) {
        int bestStrategy = node.attr(STRATEGY).toInt();
        double highestScore = node.attr(SCORE).toDouble();
        for (const Node& neighbour : node.outEdges()) {
            const double neighbourScore = neighbour.attr(SCORE).toDouble();
            if (neighbourScore > highestScore) {
                highestScore = neighbourScore;
                bestStrategy = neighbour.attr(STRATEGY).toInt();
            }
        }
        bestStrategies.emplace_back(binarize(bestStrategy));
    }

    // 3. prepare the next generation
    size_t i = 0;
    for (Node node : nodes()) {
        int s = binarize(node.attr(STRATEGY).toInt());
        s = (s == bestStrategies.at(i)) ? s : bestStrategies.at(i) + 2;
        node.setAttr(STRATEGY, s);
        ++i;
    }

    return true;
}

// 0) cooperator; 1) new cooperator
// 2) defector;   3) new defector
double PDGame::playGame(const int sX, const int sY) const
{
    switch (binarize(sX) * 2 + binarize(sY)) {
    case 0: return m_cc;    // CC : Reward for mutual cooperation
    case 1: return m_cd;    // CD : Sucker's payoff
    case 2: return m_dc;    // DC : Temptation to defect
    case 3: return m_dd;    // DD : Punishment for mutual defection
    default: qFatal("Error! strategy should be 0 or 1!");
    }
}

// transform from 0|2 -> 0 (cooperator)
//                1|3 -> 1 (defector)
int PDGame::binarize(const int strategy) const
{
    return (strategy < 2) ? strategy : strategy - 2;
}

} // evoplex
REGISTER_PLUGIN(PDGame)
#include "plugin.moc"
