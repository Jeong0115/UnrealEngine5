CREATE DEFINER=`root`@`localhost` FUNCTION `GetErrorCode`(symbol VARCHAR(100)) RETURNS int
    DETERMINISTIC
BEGIN
    DECLARE result INT;
    
    SELECT ErrorCode INTO result
    FROM errorTable
    WHERE ErrorSymbol = symbol;
    
    RETURN IFNULL(result, -1);
END

CREATE DEFINER=`root`@`localhost` PROCEDURE `ErrNoText`
(
    IN inErrorCode      INT,
    IN inErrorSymbol    VARCHAR(100),
    IN inTemp           VARCHAR(255)
)
BEGIN
    INSERT INTO rpgworld.errorTable (ErrorCode, ErrorSymbol)
        VALUES (inErrorCode, inErrorSymbol);
END