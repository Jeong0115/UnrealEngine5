
CREATE DEFINER=`root`@`localhost` PROCEDURE `SaveInventory`
(
    IN inUserID         INT, 
    IN inInventoryJson  JSON
)
BEGIN
    INSERT INTO Inventory (UserID, SlotIndex, ItemKey, ItemCount)
    SELECT inUserID, _slotIndex, _itemKey, _itemCount
    FROM JSON_TABLE
    (
        inInventoryJson, 
        "$.invenTable[*]" COLUMNS
        (
            _slotIndex  INT PATH "$._slotIndex",
            _itemKey    INT PATH "$._itemKey",
            _itemCount  INT PATH "$._itemCount"
        )
    ) AS invenTable
    ON DUPLICATE KEY UPDATE 
        ItemKey     = VALUES(ItemKey),
        ItemCount   = VALUES(ItemCount);
END


CREATE DEFINER=`root`@`localhost` PROCEDURE `SelectUserInventory`
(
    IN  inUserID    INT
)
BEGIN
    IF EXISTS (SELECT 1 FROM Inventory WHERE UserID = inUserID) THEN
        SELECT 0 As Status;
        SELECT SlotIndex, ItemKey, ItemCount
        FROM Inventory WHERE UserID = inUserID;
    ELSE
        SELECT 1 As Status, "Cannot find user id" As Message;
    END IF;
END


CREATE DEFINER=`root`@`localhost` PROCEDURE `RewardItemInventory`
(
    IN inUserID        INT, 
    IN inInventoryJson JSON
)
BEGIN
    DECLARE totalCount  INT DEFAULT 0;
    DECLARE updateCount INT DEFAULT 0;

    DECLARE Exit HANDLER FOR SQLEXCEPTION 
    BEGIN
        ROLLBACK;
        SELECT 1 AS status;
    END;

    TRUNCATE TABLE inInvenTable;

    START TRANSACTION;

    INSERT INTO inInvenTable 
    SELECT * FROM JSON_TABLE
    (
        inInventoryJson, 
        "$.invenTable[*]" COLUMNS
        (
            _slotIndex  INT PATH "$._slotIndex",
            _itemKey    INT PATH "$._itemKey",
            _itemCount  INT PATH "$._itemCount"
        )
    ) AS invenTable;

    SELECT COUNT(*) INTO totalCount FROM inInvenTable;

    UPDATE Inventory
    JOIN inInvenTable
    ON Inventory.UserID = inUserID AND Inventory.SlotIndex = inInvenTable._slotIndex
    SET 
        Inventory.ItemKey   = inInvenTable._itemKey,
        Inventory.ItemCount = Inventory.ItemCount + inInvenTable._itemCount
    WHERE 
        Inventory.ItemKey = inInvenTable._itemKey OR Inventory.ItemKey = -1;

    SELECT ROW_COUNT() INTO updateCount;

    IF updateCount < totalCount THEN
        ROLLBACK;
        SELECT 1 AS status;
    ELSE
        COMMIT;
        SELECT 0 AS status;
    END IF;

END;


CREATE DEFINER=`root`@`localhost` PROCEDURE `PurchaseItemInventory`
(
    IN inUserID        INT, 
    IN inInventoryJson JSON
)
BEGIN
    DECLARE totalCount  INT DEFAULT 0;
    DECLARE updateCount INT DEFAULT 0;

    DECLARE Exit HANDLER FOR SQLEXCEPTION 
    BEGIN
        ROLLBACK;
        SELECT 1 AS status;
    END;

    TRUNCATE TABLE inInvenTable;

    START TRANSACTION;

    INSERT INTO inInvenTable 
    SELECT * FROM JSON_TABLE
    (
        inInventoryJson, 
        "$.invenTable[*]" COLUMNS
        (
            _slotIndex  INT PATH "$._slotIndex",
            _itemKey    INT PATH "$._itemKey",
            _itemCount  INT PATH "$._itemCount"
        )
    ) AS invenTable;

    SELECT COUNT(*) INTO totalCount FROM inInvenTable;

    UPDATE Inventory
    JOIN inInvenTable
    ON Inventory.UserID = inUserID AND Inventory.SlotIndex = inInvenTable._slotIndex
    SET 
        Inventory.ItemCount = Inventory.ItemCount + inInvenTable._itemCount,
        Inventory.ItemKey = 
            CASE 
                WHEN Inventory.ItemCount + inInvenTable._itemCount = 0 THEN -1
                ELSE inInvenTable._itemKey
            END
    WHERE 
        Inventory.ItemKey = inInvenTable._itemKey OR Inventory.ItemKey = -1;

    SELECT ROW_COUNT() INTO updateCount;

    IF updateCount < totalCount THEN
        ROLLBACK;
        SELECT 1 AS status;
    ELSE
        COMMIT;
        SELECT 0 AS status;
    END IF;

END;



CREATE DEFINER=`root`@`localhost` PROCEDURE `SortInventory`
(
    IN inUserID        INT, 
    IN inInventoryJson JSON
)
BEGIN
    DECLARE totalCount  INT DEFAULT 0;
    DECLARE updateCount INT DEFAULT 0;

    DECLARE Exit HANDLER FOR SQLEXCEPTION 
    BEGIN
        ROLLBACK;
        SELECT 1 AS status;
    END;

    TRUNCATE TABLE inInvenTable;

    START TRANSACTION;

    INSERT INTO inInvenTable 
    SELECT * FROM JSON_TABLE
    (
        inInventoryJson, 
        "$.invenTable[*]" COLUMNS
        (
            _slotIndex  INT PATH "$._slotIndex",
            _itemKey    INT PATH "$._itemKey",
            _itemCount  INT PATH "$._itemCount"
        )
    ) AS invenTable;

    -- SELECT COUNT(*) INTO totalCount FROM inInvenTable;

    UPDATE Inventory
    JOIN inInvenTable
    ON Inventory.UserID = inUserID AND Inventory.SlotIndex = inInvenTable._slotIndex
    SET 
        Inventory.ItemKey   = inInvenTable._itemKey,
        Inventory.ItemCount = inInvenTable._itemCount;


    -- SELECT ROW_COUNT() INTO updateCount;

    UPDATE Inventory
    LEFT JOIN inInvenTable
    ON Inventory.UserID = inUserID AND Inventory.SlotIndex = inInvenTable._slotIndex
    SET 
        Inventory.ItemKey   = ( SELECT COLUMN_DEFAULT 
                                FROM INFORMATION_SCHEMA.COLUMNS 
                                WHERE TABLE_NAME = 'Inventory' AND COLUMN_NAME = 'ItemKey'),
        Inventory.ItemCount = ( SELECT COLUMN_DEFAULT 
                                FROM INFORMATION_SCHEMA.COLUMNS 
                                WHERE TABLE_NAME = 'Inventory' AND COLUMN_NAME = 'ItemCount')
    WHERE Inventory.UserID = inUserID AND inInvenTable._slotIndex IS NULL;

    -- IF updateCount < totalCount THEN
    --     ROLLBACK;
    --     SELECT 1 AS status;
    -- ELSE
        COMMIT;
        SELECT 0 AS status;
   -- END IF;
END;



CREATE DEFINER=`root`@`localhost` PROCEDURE `UseItemInventory`
(
    IN  inUserID        INT,
    IN  inSlotIndex     INT,
    IN  inItemKey       INT,
    IN  inUseItemCount  INT

)
BEGIN
    DECLARE result          INT;

    UPDATE Inventory
    SET 
        ItemKey = 
            CASE 
                WHEN (ItemCount - inUseItemCount) = 0 
                THEN 
                (
                    SELECT  COLUMN_DEFAULT 
                    FROM    INFORMATION_SCHEMA.COLUMNS 
                    WHERE   TABLE_NAME  = 'Inventory' 
                    AND     COLUMN_NAME = 'ItemKey'
                )
                ELSE ItemKey 
            END,
        ItemCount = ItemCount - inUseItemCount
    WHERE UserID        = inUserID
        AND SlotIndex   = inSlotIndex
        AND ItemKey     = inItemKey
        AND (ItemCount - inUseItemCount) >= 0;


    SET result = ROW_COUNT();

    IF result > 0 THEN
        SELECT 0 AS Status, "Success UseItem" AS Message;
    ELSE
        SELECT 1 AS Status, "Fail UseItem" AS Message;
    END IF;

    COMMIT;
END;