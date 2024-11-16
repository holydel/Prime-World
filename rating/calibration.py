DIFFERENCE_MODIFIER = 2.5
TEAM_DIFFERENCE_MODIFIER = 2.5
WIN_RATING = 30
LOSE_RATING = 30
START_RATING = 1100

CALIBRATION_BONUS = 2.5

import json
import math

gs = open('game_session.json')
js = json.load(gs, strict=False)

sessions = []
sessionDict = {}
sIt = 0
for session in js:
    sId = session['sessionId']
    if sId not in sessionDict:
        sessionDict[sId] = sIt
        sIt += 1
        sessions.append({'sId': sId, 'players': [], 'finished': 0})

    sessions[sessionDict[sId]]['players'].append({'uId': session['userId'], 'hId': session['heroId'],
                                             'tId': session['teamId'], 'status': session['status'],
                                             'finish': session['finish']})
    sessions[sessionDict[sId]]['finished'] += session['finish']

onlyFinishedSessions = list(filter(lambda x: x.get('players', '')[0]['status'] != 0, sessions))
ratedSessions = list(filter(lambda x: x.get('finished', '') == 10, onlyFinishedSessions))

users = {}
use = []
for session in onlyFinishedSessions:
    for player in session['players']:
        users[player['uId']] = {}
        if player['uId'] == 3850:
            use.append(player['tId'])

for session in onlyFinishedSessions:
    for player in session['players']:
        users[player['uId']][player['hId']] = START_RATING
        #users[player['uId']][0] = START_RATING

for session in onlyFinishedSessions:
    # 1. calc geometric mean rating for each team and total
    teamMeanRating = [1, 1]
    totalMeanRating = 1
    for player in session['players']:
        #teamMeanRating[player['tId'] - 1] *= users[player['uId']][0]
        #totalMeanRating *= users[player['uId']][0]
        teamMeanRating[player['tId'] - 1] *= users[player['uId']][player['hId']]
        totalMeanRating *= users[player['uId']][player['hId']]
    teamMeanRating[0] = pow(teamMeanRating[0], 1.0 / 5.0)
    teamMeanRating[1] = pow(teamMeanRating[1], 1.0 / 5.0)
    #print('Team mean rating: ' + str(teamMeanRating[0]) + ' / ' + str(teamMeanRating[1]))

    totalMeanRating = pow(totalMeanRating, 1.0 / 10.0)

    teamDiff = [teamMeanRating[1] - teamMeanRating[0], teamMeanRating[0] - teamMeanRating[1]]
    teamRatingModifier = [math.tanh(teamDiff[0] * TEAM_DIFFERENCE_MODIFIER * 0.001), math.tanh(teamDiff[1] * TEAM_DIFFERENCE_MODIFIER * 0.001)]
    #print('Team rating modifier ' + str(teamRatingModifier[0]))

    # 2. calc player individual rating
    for player in session['players']:
        playersDeltaRating = users[player['uId']][player['hId']] - totalMeanRating
        playersNormalizedRating = math.tanh(playersDeltaRating * DIFFERENCE_MODIFIER * 0.001)

        winRaw = WIN_RATING * (1 - playersNormalizedRating) / 2.0
        loseRaw = LOSE_RATING * (1 + playersNormalizedRating) / 2.0

        win = winRaw * (1 + teamRatingModifier[player['tId'] - 1])
        lose = loseRaw * (1 - teamRatingModifier[player['tId'] - 1])

        isWinner = player['status'] == player['tId']
        playerRatingBefore = users[player['uId']][player['hId']]
        if isWinner:
            users[player['uId']][player['hId']] += win
            if player['uId'] == 1853 and player['hId'] == 55:
                print('Winner:' + str(player['uId']) + '(' + str(player['tId']) + ')' + '; old rating = ' + str(playerRatingBefore) + '; new rating = ' + str(users[player['uId']][player['hId']]) + ' won ' + str(win) + '(' + str(winRaw) + ')')
        else:
            users[player['uId']][player['hId']] -= lose
            users[player['uId']][player['hId']] = max(START_RATING, users[player['uId']][player['hId']])
            if player['uId'] == 1853 and player['hId'] == 55:
                print('Loser:' + str(player['uId']) + '(' + str(player['tId']) + ')' + '; old rating = ' + str(playerRatingBefore) + '; new rating = ' + str(users[player['uId']][player['hId']]) + ' lost ' + str(lose) + '(' + str(loseRaw) + ')')

        #users[player['uId']][0] = max(users[player['uId']][0], users[player['uId']][player['hId']])
    #print(' ')

#sortedUsers = sorted(users.items(), key=lambda x:x[1][0], reverse=True)
for user in users.items():
    for hero in user[1].items():
        users[user[0]][hero[0]] = round(START_RATING + (hero[1] - START_RATING) * CALIBRATION_BONUS, 0)

#sortedUsers = sorted(users.items(), key=lambda x:x[1][0], reverse=True)
with open("rating_json.txt", "w") as outfile:
    json.dump(users, outfile)
i = 0