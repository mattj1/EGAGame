unit gamedata;
interface
uses ega;

type
	TSqrtLUT = array[0..32764] of word;

  TResources = Record
    sprite_cursor: EGASprite;
    sprite_player: EGASprite;
    sqrt: ^TSqrtLUT;
  end;

var 
    res: TResources;

implementation

begin
end.