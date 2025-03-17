CREATE DEFINER=`root`@`localhost` PROCEDURE `AcceptQuest`
(
    IN  inUserID            INT,
    IN  inQuestID           INT,
    IN  inQuestType         INT,
    IN  inClearCount        INT
)
BEGIN
    DECLARE result INT;

    INSERT INTO rpgworld.quest (UserID, QuestID, QuestType, QuestState, ClearCount, AcceptTime)
    VALUES (inUserID, inQuestID, inQuestType, 0, inClearCount, NOW());

    SET result = ROW_COUNT();

    IF result > 0 THEN
        SELECT 0 AS Status, 'Success' As Message;
    ELSE
        SELECT 1 AS Status, 'Fail' As Message;
    END IF;

    COMMIT;
END


CREATE DEFINER=`root`@`localhost` PROCEDURE `ClearQuest`
(
    IN  inUserID        INT,
    IN  inQuestID       INT
)
BEGIN
    DECLARE result          INT;
    DECLARE selectQuestType INT;

    SELECT QuestType INTO selectQuestType
    FROM rpgworld.quest 
    WHERE UserID = inUserID AND QuestID = inQuestID;

    IF selectQuestType = 1 THEN
        UPDATE rpgworld.quest
        SET QuestState = 0, ConditionCount = 0, AcceptTime = NOW()
        WHERE UserID            = inUserID
          AND QuestID           = inQuestID
          AND QuestState        = 1
          AND ConditionCount    = ClearCount;
    ELSE
        UPDATE rpgworld.quest
        SET QuestState = 2, ClearTime = NOW()
        WHERE UserID            = inUserID
          AND QuestID           = inQuestID
          AND QuestState        = 1
          AND ConditionCount    = ClearCount;
    END IF;

    SET result = ROW_COUNT();

    IF result > 0 THEN
        SELECT 0 AS Status, 'Success' AS Message;
    ELSE
        SELECT 1 AS Status, 'Fail' AS Message;
    END IF;

    COMMIT;
END;




CREATE DEFINER=`root`@`localhost` PROCEDURE `SelectUserQuest`
(
    IN  inUserID    INT
)
BEGIN
    IF EXISTS (SELECT 1 FROM Quest WHERE UserID = inUserID) THEN
        SELECT 0 As Status;
        SELECT QuestID, QuestType, QuestState, ConditionCount
        FROM Quest WHERE UserID = inUserID;
    ELSE
        SELECT 1 As Status, "Cannot find user id" As Message;
    END IF;
END


CREATE DEFINER=`root`@`localhost` PROCEDURE `UpdateQuestCondition`
(
    IN inUserID     INT, 
    IN inQuestJson  JSON
)
BEGIN
    DECLARE totalCount  INT DEFAULT 0;
    DECLARE updateCount INT DEFAULT 0;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION 
    BEGIN
        ROLLBACK;
        SELECT 1 AS status;
        SIGNAL SQLSTATE '45000';
    END;

    TRUNCATE TABLE inQuestTable;

    START TRANSACTION;

    INSERT INTO inQuestTable 
    SELECT * FROM JSON_TABLE
    (
        inQuestJson, 
        "$.questTable[*]" COLUMNS
        (
            _questID            INT PATH "$._questId",
            _questType          INT PATH "$._questType",
            _questState         INT PATH "$._questState",
            _addConditionCount  INT PATH "$._conditionCount"
        )
    ) AS questTable;

    SELECT COUNT(*) INTO totalCount FROM inQuestTable;

    UPDATE Quest
    JOIN inQuestTable
        ON Quest.UserID = inUserID
            AND Quest.QuestID       = inQuestTable._questID
            AND Quest.QuestType     = inQuestTable._questType
            AND Quest.QuestState    = inQuestTable._questState
    SET Quest.ConditionCount =
        CASE
            WHEN Quest.ConditionCount + inQuestTable._addConditionCount >= Quest.ClearCount
                THEN Quest.ClearCount
            ELSE Quest.ConditionCount + inQuestTable._addConditionCount
        END;

    SELECT ROW_COUNT() INTO updateCount;

    UPDATE Quest
    JOIN inQuestTable
        ON Quest.UserID = inUserID
            AND Quest.QuestID       = inQuestTable._questID
            AND Quest.QuestType     = inQuestTable._questType
            AND Quest.QuestState    = inQuestTable._questState
    SET Quest.QuestState = 1 WHERE Quest.ConditionCount >= Quest.ClearCount;

    IF updateCount < totalCount THEN
        ROLLBACK;
        SELECT 1 AS status;
    ELSE
        COMMIT;
        SELECT 0 AS status;
    END IF;

END