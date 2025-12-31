unit gamedata;
interface
uses 
  {$ifdef PLATFORM_DESKTOP}
  raylib,
  {$endif}
  ega, res;

type
	TSqrtLUT = array[0..32764] of word;

  TResources = Record
    sprites: array[0..SPRITE_STATE_MAX] of EGASprite;
    sqrt: ^TSqrtLUT;

  {$ifdef PLATFORM_DESKTOP}
    raylib_font: TImage;
    raylib_tiles: TImage;
  {$endif}

  end;

var 
    assets: TResources;

implementation

begin
end.