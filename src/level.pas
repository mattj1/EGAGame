unit level;

interface

const TILEFLAG_SOLID = 1;

type
  PTile = ^TTile;
  TTile = record
    t:word;
    flags: word;
  end;

  TTileRef = record
    x, y: word;
    tile: ^TTile;
  end;

  TileArray = array[0..1024] of TTile;

  PMap = ^TMap;
  TMap = record
    width, height: word;
    tiles: ^TileArray;
  end;

var
  currentMap: PMap;

function Map_Alloc(width, height: word; var map: TMap): boolean;
function Map_TileAt(var map: TMap; x, y: word): PTile;

implementation

function Map_Alloc(width, height: word; var map: TMap): boolean;
begin
  Map_Alloc := True;
  map.width := width;
  map.height := height;
  GetMem(map.tiles, sizeof(TTile) * width * height);

  FillChar(map.tiles^, sizeof(TTile) * width * height, 0);

  currentMap := @map;
end;

function Map_TileAt(var map: TMap; x, y: word): PTile;
begin
  Map_TileAt := @map.tiles^[y * map.width + x];
end;

begin
end.

