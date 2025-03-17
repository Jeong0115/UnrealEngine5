CREATE DEFINER=`root`@`localhost` PROCEDURE `CreateAccount`
(
    IN inUserName       VARCHAR(64),
    IN inUserPassword   VARCHAR(64)
)
BEGIN
    DECLARE newId INT;

    IF EXISTS (SELECT 1 FROM account WHERE UserName = inUserName) THEN
        SELECT 1 AS Status, 'UserName already exists.' AS Message;
    ELSE
        INSERT INTO account (UserName, UserPassword)
        VALUES (inUserName, SHA2(inUserPassword, 256));

        SET newId = LAST_INSERT_ID();
        CALL InitializeNewAccount(newId);

        SELECT 0 AS Status, CAST(newId AS CHAR) AS Message;
    END IF;

    COMMIT;
END;

CREATE DEFINER=`root`@`localhost` PROCEDURE `LoginAccount`
(
    IN inUserName       VARCHAR(64),
    IN inUserPassword   VARCHAR(64)
)
BEGIN
    DECLARE Id INT DEFAULT NULL;
    
    SELECT UserId INTO Id 
    FROM account 
    WHERE UserName = inUserName AND UserPassword = SHA2(inUserPassword, 256)
    LIMIT 1;

    IF Id IS NOT NULL THEN
        SELECT 0 AS Status, CAST(Id AS CHAR) AS Message;
    ELSE
        SELECT 1 AS Status, 'LOGIN FAILED' AS Message;
    END IF;
END;


CREATE DEFINER=`root`@`localhost` PROCEDURE `InitializeNewAccount`
(
    IN inUserID INT
)
BEGIN
    INSERT INTO Quest (UserID, QuestID, QuestType, QuestState, ClearCount, AcceptTime)
    VALUES (inUserID, 1000, 0, 0, 10, NOW());

    INSERT INTO Quest (UserID, QuestID, QuestType, QuestState, ClearCount, AcceptTime)
    VALUES (inUserID, 2000, 1, 0, 10, NOW());

    INSERT INTO Quest (UserID, QuestID, QuestType, QuestState, ClearCount, AcceptTime)
    VALUES (inUserID, 3000, 1, 0, 10000, NOW());

    INSERT INTO Quest (UserID, QuestID, QuestType, QuestState, ClearCount, AcceptTime)
    VALUES (inUserID, 4000, 0, 0, 100, NOW());

    INSERT INTO inventory (UserID, SlotIndex)
    SELECT inUserID, newSlotIndex 
    FROM 
    (
        WITH RECURSIVE slotNumbers AS 
        (
            SELECT 0 AS newSlotIndex
            UNION ALL
            SELECT newSlotIndex + 1 FROM slotNumbers WHERE newSlotIndex < 31
        )
        SELECT newSlotIndex FROM slotNumbers
    ) AS slotList;

    INSERT INTO GameOptions (UserID, InventorySize) Value(inUserID, 32);

    COMMIT;
END


CREATE DEFINER=`root`@`localhost` PROCEDURE `DeleteAccount`
(
    IN inUserId INT
)
BEGIN
	DELETE FROM Quest       WHERE UserID = inUserId;
    DELETE FROM Inventory   WHERE UserID = inUserId;
    DELETE FROM GameOptions WHERE UserID = inUserId;
    DELETE FROM Account     WHERE UserID = inUserId;

    COMMIT;
END

CREATE DEFINER=`root`@`localhost` PROCEDURE `InitAccount`
(
    IN inUserId INT
)
BEGIN
	DELETE FROM Quest       WHERE UserID = inUserId;
    DELETE FROM Inventory   WHERE UserID = inUserId;
    DELETE FROM GameOptions WHERE UserID = inUserId;

	CALL InitializeNewAccount(inUserId);

    COMMIT;
END