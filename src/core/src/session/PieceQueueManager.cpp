#include "PieceQueueManager.h"

#include "loguru.hpp"

namespace core
{
PieceQueueManager::PieceQueueManager(libs::collection::ThreadSafeQueue<int>& piecesQueueInit)
    : piecesQueue{piecesQueueInit}
{
}

void PieceQueueManager::addPieceIdoQueue(const int& pieceId)
{
    piecesQueue.push(pieceId);
}

std::optional<int> PieceQueueManager::getPieceIdFromQueue() const
{
    std::lock_guard<std::mutex> guard(lock);

    if (piecesQueue.size() < 10)
    {
        const auto piecesIds = piecesQueue.popAll();

        for (const auto& pieceId : piecesIds)
        {
            piecesQueue.push(pieceId);
        }
    }

    return piecesQueue.pop();
}

}
