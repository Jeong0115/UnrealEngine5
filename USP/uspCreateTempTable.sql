CREATE DEFINER=`root`@`localhost` PROCEDURE `CreateTempTable` ()
BEGIN
    DROP TEMPORARY TABLE IF EXISTS inQuestTable;
    CREATE TEMPORARY TABLE inQuestTable 
    (
        _questID            INT,
        _questType          INT,
        _questState         INT,
        _addConditionCount  INT
    );

    DROP TEMPORARY TABLE IF EXISTS inInvenTable;
    CREATE TEMPORARY TABLE inInvenTable 
    (
        _slotIndex  INT,
        _itemKey    INT,
        _itemCount  INT
    );
    
	SET autocommit = 0;
END