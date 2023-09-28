//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include "game/physics.hpp"

#include "game/game.hpp"
#include "game/sprites.hpp"
#include "game/gifts.hpp"
#include "settings.hpp"
#include "gfx/particles.hpp"
#include "gfx/effect.hpp"
#include "gfx/text.hpp"
#include "language.hpp"
#include "sfx.hpp"
#include "system.hpp"
#include "episode/episodeclass.hpp"
#include "gui.hpp"

#include "engine/types.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"

#include <cstring>

static double sprite_x;
static double sprite_y;
static double sprite_a;
static double sprite_b;

static double sprite_vasen;
static double sprite_oikea;
static double sprite_yla;
static double sprite_ala;

static int sprite_leveys;
static int sprite_korkeus;

static bool oikealle;
static bool vasemmalle;
static bool ylos;
static bool alas;

static bool in_water;

static double max_speed;

static PK2BLOCK Block_Get(u32 x, u32 y) {

	PK2BLOCK block;
	//memset(&block, 0, sizeof(block));

	// Outside the screen
	if (x >= PK2MAP_MAP_WIDTH || y >= PK2MAP_MAP_HEIGHT) {
		
		block.koodi  = 255;
		block.tausta = true;
		block.vasen  = x*32;
		block.oikea  = x*32 + 32;
		block.yla    = y*32;
		block.ala    = y*32 + 32;
		block.water  = false;
		block.border = true;

		block.vasemmalle = 0;
		block.oikealle = 0;
		block.ylos = 0;
		block.alas = 0;

		return block;

	}

	u8 i = Game->map.foreground_tiles[x+y*PK2MAP_MAP_WIDTH];

	if (i<150) { //If it is ground

		block        = Game->block_types[i];
		block.vasen  = x*32+Game->block_types[i].vasen;
		block.oikea  = x*32+32+Game->block_types[i].oikea;
		block.yla    = y*32+Game->block_types[i].yla;
		block.ala    = y*32+32+Game->block_types[i].ala;

	} else { //If it is sky - Need to reset
	
		block.koodi  = 255;
		block.tausta = true;
		block.vasen  = x*32;
		block.oikea  = x*32 + 32;
		block.yla    = y*32;
		block.ala    = y*32 + 32;
		block.water  = false;

		block.vasemmalle = 0;
		block.oikealle = 0;
		block.ylos = 0;
		block.alas = 0;
	
	}

	i = Game->map.background_tiles[x+y*PK2MAP_MAP_WIDTH];

	if (i > 131 && i < 140)
		block.water = true;

	block.border = Game->map.edges[x+y*PK2MAP_MAP_WIDTH];

	return block;
}

static void Check_SpriteBlock(SpriteClass* sprite, const PK2BLOCK &block) {

	//left and right
	if (sprite_yla < block.ala && sprite_ala-1 > block.yla){
		if (sprite_oikea+sprite_a-1 > block.vasen && sprite_vasen+sprite_a < block.oikea){
			// Tutkitaan onko sprite menossa oikeanpuoleisen palikan sis��n.
			if (sprite_oikea+sprite_a < block.oikea){
				// Onko block sein�?
				if (block.oikealle == BLOCK_WALL){
					oikealle = false;
					if (block.koodi == BLOCK_LIFT_HORI)
						sprite_x = block.vasen - sprite_leveys/2;
				}
			}

			if (sprite_vasen+sprite_a > block.vasen){
				if (block.vasemmalle == BLOCK_WALL){
					vasemmalle = false;

					if (block.koodi == BLOCK_LIFT_HORI)
						sprite_x = block.oikea + sprite_leveys/2;

				}
			}
		}
	}

	sprite_vasen = sprite_x-sprite_leveys/2;
	sprite_oikea = sprite_x+sprite_leveys/2;

	//ceil and floor

	if (sprite_vasen < block.oikea && sprite_oikea-1 > block.vasen){
		if (sprite_ala+sprite_b-1 >= block.yla && sprite_yla+sprite_b <= block.ala){
			if (sprite_ala+sprite_b-1 <= block.ala){
				if (block.alas == BLOCK_WALL){
					alas = false;

					if (block.koodi == BLOCK_LIFT_VERT)
						sprite_y = block.yla - sprite_korkeus /2;

					if (sprite_ala-1 >= block.yla && sprite_b >= 0)
						if (block.koodi != BLOCK_LIFT_HORI)
							sprite_y = block.yla - sprite_korkeus /2;
				}
			}

			if (sprite_yla+sprite_b > block.yla){
				if (block.ylos == BLOCK_WALL){
					ylos = false;

					if (sprite_yla < block.ala)
						if (block.koodi != BLOCK_LIFT_HORI)
							sprite->crouched = true;
				}
			}
		}
	}
}

void Check_MapBlock(SpriteClass* sprite, PK2BLOCK block) {

	//If sprite is in the block
	if (sprite_x <= block.oikea && sprite_x >= block.vasen && sprite_y <= block.ala && sprite_y >= block.yla){

		/**********************************************************************/
		/* Examine if block is water background                               */
		/**********************************************************************/
		if (block.water)
			sprite->in_water = true;
		else
			sprite->in_water = false;

		/**********************************************************************/
		/* Examine if it touches the fire                                     */
		/**********************************************************************/
		if (block.koodi == BLOCK_FIRE && Game->button1 == 0 && sprite->damage_timer == 0){
			sprite->saatu_vahinko = 2;
			sprite->saatu_vahinko_tyyppi = DAMAGE_FIRE;
		}

		/**********************************************************************/
		/* Examine if block is hideway (unused)                               */
		/**********************************************************************/
		if (block.koodi == BLOCK_HIDEOUT)
			sprite->hidden = true;
		else
			sprite->hidden = false;
		

		/**********************************************************************/
		/* Examine if block is the exit                                       */
		/**********************************************************************/
		if (block.koodi == BLOCK_EXIT) {
			if ((!Game->chick_mode && sprite->player != 0) || sprite->HasAI(AI_CHICK))
				Game->Finnish();
		}
	}

	//If sprite is thouching the block
	if (sprite_vasen <= block.oikea-4 && sprite_oikea >= block.vasen+4 && sprite_yla <= block.ala && sprite_ala >= block.yla+16){
		/**********************************************************************/
		/* Examine if it touches the fire                                     */
		/**********************************************************************/
		if (block.koodi == BLOCK_FIRE && Game->button1 == 0 && sprite->damage_timer == 0){
			sprite->saatu_vahinko = 2;
			sprite->saatu_vahinko_tyyppi = DAMAGE_FIRE;
		}
	}

	//Examine if there is a block on bottom
	if ((block.koodi<80 || block.koodi>139) && block.koodi != BLOCK_BARRIER_DOWN && block.koodi < 150){
		int mask_index = (int)(sprite_x+sprite_a) - block.vasen;

		if (mask_index < 0)
			mask_index = 0;

		if (mask_index > 31)
			mask_index = 31;

		block.yla += Game->block_masks[block.koodi].alas[mask_index];

		if (block.yla >= block.ala-2)
			block.alas = BLOCK_BACKGROUND;

		block.ala -= Game->block_masks[block.koodi].ylos[mask_index];
	}

	//If sprite is thouching the block (again?)
	if (sprite_vasen <= block.oikea+2 && sprite_oikea >= block.vasen-2 && sprite_yla <= block.ala && sprite_ala >= block.yla){
		/**********************************************************************/
		/* Examine if it is a key and touches lock wall                       */
		/**********************************************************************/
		if (block.koodi == BLOCK_LOCK && sprite->prototype->can_open_locks){
			Game->map.foreground_tiles[block.vasen/32+(block.yla/32)*PK2MAP_MAP_WIDTH] = 255;
			Game->map.Calculate_Edges();

			sprite->removed = true;

			if (sprite->prototype->how_destroyed != FX_DESTRUCT_EI_TUHOUDU) {
				Game->keys--;
				if (Game->keys < 1)
					Game->Open_Locks();
			}

			Effect_Explosion(block.vasen+16, block.yla+10, 0);
			Play_GameSFX(open_locks_sound,100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, false);
		}

		/**********************************************************************/
		/* Make wind effects                                                  */
		/**********************************************************************/
		if (block.koodi == BLOCK_DRIFT_LEFT && vasemmalle)
			sprite_a -= 0.02;

		if (block.koodi == BLOCK_DRIFT_RIGHT && oikealle)
			sprite_a += 0.02;	//0.05

		/*********************************************************************/
		/* Examine if sprite is on the border to fall                        */
		/*********************************************************************/
		if (block.border && sprite->jump_timer <= 0 && sprite_y < block.ala && sprite_y > block.yla){
			/* && sprite_ala <= block.ala+2)*/ // onko sprite tullut borderlle
			if (sprite_vasen > block.vasen)
				sprite->reuna_vasemmalla = true;

			if (sprite_oikea < block.oikea)
				sprite->reuna_oikealla = true;
		}
	}

	//Examine walls on left and right

	if (sprite_yla < block.ala && sprite_ala-1 > block.yla) {
		if (sprite_oikea+sprite_a-1 > block.vasen && sprite_vasen+sprite_a < block.oikea) {
			// Examine whether the sprite going in the right side of the block.
			if (sprite_oikea+sprite_a < block.oikea) {
				// Onko block sein�?
				if (block.oikealle == BLOCK_WALL) {
					oikealle = false;

					if (block.koodi == BLOCK_LIFT_HORI)
						sprite_x = block.vasen - sprite_leveys/2;
				}
			}
			// Examine whether the sprite going in the left side of the block.
			if (sprite_vasen+sprite_a > block.vasen) {
				if (block.vasemmalle == BLOCK_WALL) {
					vasemmalle = false;

					if (block.koodi == BLOCK_LIFT_HORI)
						sprite_x = block.oikea + sprite_leveys/2;

				}
			}
		}
	}

	sprite_vasen = sprite_x - sprite_leveys/2;
	sprite_oikea = sprite_x + sprite_leveys/2;

	//Examine walls on up and down

	if (sprite_vasen < block.oikea && sprite_oikea-1 > block.vasen) { //Remove the left and right blocks
		if (sprite_ala+sprite_b-1 >= block.yla && sprite_yla+sprite_b <= block.ala) { //Get the up and down blocks
			if (sprite_ala+sprite_b-1 <= block.ala) { //Just in the sprite's foot
				if (block.alas == BLOCK_WALL) { //If it is a wall
					alas = false;
					if (block.koodi == BLOCK_LIFT_VERT)
						sprite_y = block.yla - sprite_korkeus /2;

					if (sprite_ala-1 >= block.yla && sprite_b >= 0) {
						//sprite_y -= sprite_ala - block.yla;
						if (block.koodi != BLOCK_LIFT_HORI) {
							sprite_y = block.yla - sprite_korkeus /2;
						}
					}

					if (sprite->kytkinpaino >= 1) { // Sprite can press the buttons
						if (block.koodi == BLOCK_BUTTON1 && Game->button1 == 0) {
							Game->button1 = Game->map.button1_time;
							Game->button_vibration = 64;
							Play_GameSFX(switch_sound, 100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}

						if (block.koodi == BLOCK_BUTTON2 && Game->button2 == 0) {
							Game->button2 = Game->map.button2_time;
							Game->button_vibration = 64;
							Play_GameSFX(switch_sound, 100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}

						if (block.koodi == BLOCK_BUTTON3 && Game->button3 == 0) {
							Game->button3 = Game->map.button3_time;
							Game->button_vibration = 64;
							Play_GameSFX(switch_sound, 100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, false);
							PInput::Vibrate(1000);
						}
					}

				}
			}

			if (sprite_yla+sprite_b > block.yla) {
				if (block.ylos == BLOCK_WALL) {
					ylos = false;

					if (sprite_yla < block.ala) {
						if (block.koodi == BLOCK_LIFT_VERT && sprite->crouched) {
							sprite->saatu_vahinko = 2;
							sprite->saatu_vahinko_tyyppi = DAMAGE_IMPACT;
						}

						if (block.koodi != BLOCK_LIFT_HORI) {
							//if (sprite->crouched)
							//	sprite_y = block.ala + sprite_korkeus /2;

							sprite->crouched = true;
						}
					}
				}
			}
		}
	}
}

void SpriteOnDeath(SpriteClass* sprite){
	int how_destroyed = sprite->prototype->how_destroyed;
	if (sprite->HasAI(AI_CHICK)){
		Game->game_over = true;
		key_delay = 50; //TODO - reduce
	} // Killed the chick
	if (sprite->prototype->bonus != nullptr && sprite->prototype->bonuses_number > 0)
		if (sprite->prototype->bonus_always || rand()%4 == 1)
			for (int bi=0; bi<sprite->prototype->bonuses_number; bi++)
				Sprites_add(sprite->prototype->bonus,0,sprite_x-11+(10-rand()%20),
									sprite_ala-16-(10+rand()%20), sprite, true);

	if (sprite->HasAI(AI_VAIHDA_KALLOT_JOS_TYRMATTY) && !sprite->HasAI(AI_VAIHDA_KALLOT_JOS_OSUTTU))
		Game->Change_SkullBlocks();

	if (how_destroyed >= FX_DESTRUCT_ANIMAATIO)
		how_destroyed -= FX_DESTRUCT_ANIMAATIO;
	else
		sprite->removed = true;

	Effect_Destruction(how_destroyed, (u32)sprite->x, (u32)sprite->y);
	Play_GameSFX(sprite->prototype->sounds[SOUND_DESTRUCTION],100, (int)sprite->x, (int)sprite->y,
					sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

	if (sprite->HasAI(AI_UUSI_JOS_TUHOUTUU)) {
		Sprites_add(sprite->prototype, 0, 
			sprite->orig_x - sprite->prototype->width,
			sprite->orig_y - sprite->prototype->height,
			sprite, false);
	}

	if (sprite->prototype->type == TYPE_GAME_CHARACTER && sprite->prototype->score != 0){
		Fadetext_New(fontti2,std::to_string(sprite->prototype->score),(int)sprite->x-8,(int)sprite->y-8,80);
		Game->score_increment += sprite->prototype->score;
	}
}

int UpdateSprite(SpriteClass* sprite){
	
	if (!sprite->prototype)
		return -1;

	// Save values
	sprite_x = sprite->x;
	sprite_y = sprite->y;
	sprite_a = sprite->a;
	sprite_b = sprite->b;

	sprite_leveys  = sprite->prototype->width;
	sprite_korkeus = sprite->prototype->height;

	sprite_vasen = sprite_x - sprite_leveys  / 2;
	sprite_oikea = sprite_x + sprite_leveys  / 2;
	sprite_yla   = sprite_y - sprite_korkeus / 2;
	sprite_ala   = sprite_y + sprite_korkeus / 2;

	max_speed = sprite->prototype->max_speed;

	in_water = sprite->in_water;

	oikealle	 = true,
	vasemmalle	 = true,
	ylos		 = true,
	alas		 = true;

	sprite->crouched = false;

	sprite->reuna_vasemmalla = false;
	sprite->reuna_oikealla = false;


	/* Pistet��n vauhtia tainnutettuihin spriteihin */
	if (sprite->energy < 1)
		max_speed = 3;

	// Calculate the remainder of the sprite towards

	if (sprite->attack1_timer > 0)
		sprite->attack1_timer--;

	if (sprite->attack2_timer > 0)
		sprite->attack2_timer--;

	if (sprite->charging_timer > 0)	// aika kahden ampumisen (munimisen) v�lill�
		sprite->charging_timer --;

	if (sprite->mutation_timer > 0)	// time of mutation
		sprite->mutation_timer --;

	/*****************************************************************************************/
	/* Player-sprite and its controls                                                        */
	/*****************************************************************************************/

	bool add_speed = true;
	bool gliding = false;

	bool swimming = sprite->in_water && (sprite->HasAI(AI_SWIMMING) || sprite->HasAI(AI_MAX_SPEED_SWIMMING));
	bool max_speed_available = sprite->HasAI(AI_MAX_SPEED_PLAYER) ||
		(swimming && sprite->HasAI(AI_MAX_SPEED_SWIMMING)) ||
		(sprite->super_mode_timer > 0 && sprite->HasAI(AI_MAX_SPEED_PLAYER_ON_SUPER));
	
	/*swimming = sprite->in_water;
	max_speed_available = sprite->in_water;
	sprite->prototype->can_swim = true;*/

	if (sprite->player != 0 && sprite->energy > 0){
		/* SLOW WALK */
		if (PInput::Keydown(Input->walk_slow)
		|| Gui_pad_button == 1 || Gui_pad_button == 3)
			add_speed = false;

		/* ATTACK 1 */
		if ((PInput::Keydown(Input->attack1) || Gui_egg) && sprite->charging_timer == 0 && sprite->ammo1 != nullptr)
			sprite->attack1_timer = sprite->prototype->attack1_time;
		/* ATTACK 2 */
		else if ((PInput::Keydown(Input->attack2) || Gui_doodle) && sprite->charging_timer == 0 && sprite->ammo2 != nullptr)
				sprite->attack2_timer = sprite->prototype->attack2_time;

		/* CROUCH */
		sprite->crouched = false;
		bool axis_couch = (Input == &Settings.joystick) && (PInput::GetAxis(1) > 0.5);
		if ((PInput::Keydown(Input->down) || Gui_down || axis_couch) && !sprite->alas) {
			sprite->crouched = true;
			sprite_yla += sprite_korkeus/1.5;
		}

		/* NAVIGATING*/
		int navigation = 0;

		if (Input == &Settings.joystick)
			navigation = PInput::GetAxis(0) * 100;

		if (Gui_pad_button == 0 || Gui_pad_button == 1)
			navigation = -100;
		else if (Gui_pad_button == 3 || Gui_pad_button == 4)
			navigation = 100;

		if (PInput::Keydown(Input->right))
			navigation = 100;
		
		if (PInput::Keydown(Input->left))
			navigation = -100;

		double a_lisays = 0.04;//0.08;

		if (add_speed) {
			if (rand()%20 == 1 && sprite->animation_index == ANIMATION_WALKING) // Draw dust
				Particles_New(PARTICLE_DUST_CLOUDS,sprite_x-8,sprite_ala-8,0.25,-0.25,40,0,0);

			a_lisays += 0.09;//0.05
		}

		if (sprite->alas)
			a_lisays /= 1.5;//2.0

		a_lisays *= double(navigation) / 100;

		if (max_speed_available)
			a_lisays *= max_speed;
				
		if (navigation > 0)
			sprite->flip_x = false;
		else if (navigation < 0)
			sprite->flip_x = true;

		if (sprite->crouched)	// Slow when couch
			a_lisays /= 10;

		sprite_a += a_lisays;

		/* JUMPING */
		if (sprite->prototype->weight > 0 && !swimming) {
			if (PInput::Keydown(Input->jump) || Gui_up) {
				if (!sprite->crouched) {
					if (sprite->jump_timer == 0)
						Play_GameSFX(jump_sound, 100, (int)sprite_x, (int)sprite_y,
									  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

					if (sprite->jump_timer <= 0)
						sprite->jump_timer = 1; //10;
				}
			} else {
				if (sprite->jump_timer > 0 && sprite->jump_timer < 45)
					sprite->jump_timer = 55;
			}

			/* dripping quietly down */
			bool axis_up = (Input == &Settings.joystick) && (PInput::GetAxis(1) < -0.5);
			if ((PInput::Keydown(Input->jump) || Gui_up || axis_up) && sprite->jump_timer >= 150/*90+20*/ &&
				sprite->prototype->can_glide)
				gliding = true;
		}
		/* MOVING UP AND DOWN */
		else { // if the player sprite-weight is 0 - like birds

			double speed = 0.15;
			if (max_speed_available)
				speed *= max_speed;

			if (PInput::Keydown(Input->jump) || Gui_up)
				sprite_b -= speed;

			if (PInput::Keydown(Input->down) || Gui_down)
				sprite_b += speed;

			sprite->jump_timer = 0;
		}

		/* AI */
		for(const int& sprite_ai:sprite->prototype->AI_v)
			switch (sprite_ai){
			
			case AI_CHANGE_WHEN_ENERGY_UNDER_2:
				if (sprite->prototype->transformation != nullptr)
					sprite->AI_Change_When_Energy_Under_2(sprite->prototype->transformation);
			break;
			
			case AI_CHANGE_WHEN_ENERGY_OVER_1:
			if (sprite->prototype->transformation != nullptr)
				if (sprite->AI_Change_When_Energy_Over_1(sprite->prototype->transformation)==1)
					Effect_Destruction(FX_DESTRUCT_SAVU_HARMAA, (u32)sprite->x, (u32)sprite->y);
			break;
			
			case AI_MUUTOS_AJASTIN:
				if (sprite->prototype->transformation != nullptr)
					sprite->AI_Muutos_Ajastin(sprite->prototype->transformation);
			break;
			
			case AI_DAMAGED_BY_WATER:
				sprite->AI_Damaged_by_Water();
			break;
			
			case AI_MUUTOS_JOS_OSUTTU:
				if (sprite->prototype->transformation != nullptr)
					sprite->AI_Muutos_Jos_Osuttu(sprite->prototype->transformation);
			break;

			default: break;
			}

		/* It is not acceptable that a player is anything other than the game character */
		if (sprite->prototype->type != TYPE_GAME_CHARACTER)
			sprite->energy = 0;
	}

	/*****************************************************************************************/
	/* Jump                                                                                  */
	/*****************************************************************************************/

	bool hyppy_maximissa = sprite->jump_timer >= 90;

	// Jos ollaan hyp�tty / ilmassa:
	if (sprite->jump_timer > 0) {
		if (sprite->jump_timer < 50-sprite->prototype->max_jump)
			sprite->jump_timer = 50-sprite->prototype->max_jump;

		if (sprite->jump_timer < 10)
			sprite->jump_timer = 10;

		if (!hyppy_maximissa) {
		// sprite_b = (sprite->prototype->max_jump/2 - sprite->jump_timer/2)/-2.0;//-4
		   sprite_b = -sin_table(sprite->jump_timer)/8;//(sprite->prototype->max_jump/2 - sprite->jump_timer/2)/-2.5;
			if (sprite_b > sprite->prototype->max_jump){
				sprite_b = sprite->prototype->max_jump/10.0;
				sprite->jump_timer = 90 - sprite->jump_timer;
			}

		}

		if (sprite->jump_timer < 180)
			sprite->jump_timer += 2;
	}

	if (sprite->jump_timer < 0)
		sprite->jump_timer++;

	if (sprite_b > 0 && !hyppy_maximissa)
		sprite->jump_timer = 90;//sprite->prototype->max_jump*2;

	/*****************************************************************************************/
	/* Hit recovering                                                                        */
	/*****************************************************************************************/

	if (sprite->damage_timer > 0)
		sprite->damage_timer--;

	/*****************************************************************************************/
	/* Timers                                                                                */
	/*****************************************************************************************/

	if (sprite->invisible_timer > 0) {
	
		sprite->invisible_timer--;

	}
	
	if (sprite->super_mode_timer > 0) {

		sprite->super_mode_timer--;
		if (Player_Sprite->super_mode_timer == 0)
			PSound::resume_music();
	
	}

	/*****************************************************************************************/
	/* Gravity effect                                                                        */
	/*****************************************************************************************/
	
	if (sprite->weight != 0 && (sprite->jump_timer <= 0 || sprite->jump_timer >= 45) && !swimming)
		sprite_b += sprite->weight/1.25;// + sprite_b/1.5;

	if (gliding && sprite_b > 0) // If gliding
		sprite_b /= 1.3;//1.5;//3

	/*****************************************************************************************/
	/* Speed limits                                                                          */
	/*****************************************************************************************/

	if (sprite_b > 4.0)//4
		sprite_b = 4.0;//4

	if (sprite_b < -4.0)
		sprite_b = -4.0;

	//Limit speed 1
	if (sprite_a > max_speed)
		sprite_a = max_speed;

	if (sprite_a < -max_speed)
		sprite_a = -max_speed;

	/*****************************************************************************************/
	/* Blocks colision -                                                                     */
	/*****************************************************************************************/

	if (sprite->prototype->makes_sounds){ //Find the tiles that the sprite occupies

		int palikat_x_lkm = (int)((sprite_leveys) /32)+4; //Number of blocks
		int palikat_y_lkm = (int)((sprite_korkeus)/32)+4;

		int map_vasen = (int)(sprite_vasen) / 32; //Position in tile map
		int map_yla   = (int)(sprite_yla)   / 32;

		/*****************************************************************************************/
		/* Going through the blocks around the sprite->                                           */
		/*****************************************************************************************/

		//palikat_lkm = palikat_y_lkm*palikat_x_lkm;
		for (int y = 0; y < palikat_y_lkm; y++){
			for (int x = 0; x < palikat_x_lkm; x++) {
				int p = x + y*palikat_x_lkm;
				if ( p < 300 )
					if (!(sprite == Player_Sprite && dev_mode && PInput::Keydown(PInput::Y))){
						PK2BLOCK block = Block_Get(map_vasen+x-1,map_yla+y-1);
						Check_MapBlock(sprite, block);
					}
			}
		}
	}
	/*****************************************************************************************/
	/* If the sprite is under water                                                          */
	/*****************************************************************************************/

	if (sprite->in_water) {

		if (!sprite->prototype->can_swim || sprite->energy < 1) {

			sprite_b /= 2.0;
			sprite_a /= 1.05;

			if (sprite->jump_timer > 0 && sprite->jump_timer < 90)
				sprite->jump_timer--;
		}

		if (rand()%80 == 1)
			Particles_New(PARTICLE_SPARK,sprite_x-4,sprite_y,0,-0.5-rand()%2,rand()%30+30,0,32);
	}

	if (in_water != sprite->in_water) { // Sprite comes in or out from water
		Effect_Splash(sprite_x, sprite_y, 32);
		Play_GameSFX(splash_sound, 100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, true);
	}

	/*****************************************************************************************/
	/* Sprite weight                                                                         */
	/*****************************************************************************************/

	sprite->weight = sprite->initial_weight;
	sprite->kytkinpaino = sprite->weight;

	if (sprite->energy < 1 && sprite->weight == 0) // Fall when is death
		sprite->weight = 1;

	/*****************************************************************************************/
	/* Sprite collision with other sprites                                                   */
	/*****************************************************************************************/

	int how_destroyed;
	double sprite2_yla; // kyykistymiseen liittyv�
	PK2BLOCK spritepalikka;

	//Compare this sprite with every sprite in the game
	for (SpriteClass* sprite2 : Sprites_List) {
		if (sprite2 != sprite && sprite2->active && !sprite2->removed) {
			if (sprite2->crouched)
				sprite2_yla = sprite2->prototype->height / 3;//1.5;
			else
				sprite2_yla = 0;

			if (sprite2->prototype->is_wall && sprite->prototype->makes_sounds) { //If there is a block sprite active

				if (sprite_x-sprite_leveys/2 +sprite_a  <= sprite2->x + sprite2->prototype->width /2 &&
					sprite_x+sprite_leveys/2 +sprite_a  >= sprite2->x - sprite2->prototype->width /2 &&
					sprite_y-sprite_korkeus/2+sprite_b <= sprite2->y + sprite2->prototype->height/2 &&
					sprite_y+sprite_korkeus/2+sprite_b >= sprite2->y - sprite2->prototype->height/2)
				{
					spritepalikka.koodi = 0;
					spritepalikka.ala   = (int)sprite2->y + sprite2->prototype->height/2;
					spritepalikka.oikea = (int)sprite2->x + sprite2->prototype->width/2;
					spritepalikka.vasen = (int)sprite2->x - sprite2->prototype->width/2;
					spritepalikka.yla   = (int)sprite2->y - sprite2->prototype->height/2;

					spritepalikka.water  = false;

					spritepalikka.tausta = false;

					spritepalikka.oikealle   = BLOCK_WALL;
					spritepalikka.vasemmalle = BLOCK_WALL;
					spritepalikka.ylos       = BLOCK_WALL;
					spritepalikka.alas       = BLOCK_WALL;

					if (!sprite->prototype->is_wall){
						if (!sprite2->prototype->is_wall_down)
							spritepalikka.alas = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_up)
							spritepalikka.ylos = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_right)
							spritepalikka.oikealle = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_left)
							spritepalikka.vasemmalle = BLOCK_BACKGROUND;
					}

					if (sprite2->a > 0)
						spritepalikka.koodi = BLOCK_LIFT_HORI;

					if (sprite2->b > 0)
						spritepalikka.koodi = BLOCK_LIFT_VERT;

					if (!(sprite == Player_Sprite && dev_mode && PInput::Keydown(PInput::Y)))
						Check_SpriteBlock(sprite, spritepalikka); //Colision sprite and sprite block
				}
			}

			if (sprite_x <= sprite2->x + sprite2->prototype->width /2 &&
				sprite_x >= sprite2->x - sprite2->prototype->width /2 &&
				sprite_y/*yla*/ <= sprite2->y + sprite2->prototype->height/2 &&
				sprite_y/*ala*/ >= sprite2->y - sprite2->prototype->height/2 + sprite2_yla)
			{
				// sprites with same index change directions when touch
				if (sprite->prototype == sprite2->prototype &&
					sprite2->energy > 0/* && sprite->player == 0*/)
				{
					if (sprite->x < sprite2->x)
						oikealle = false;
					if (sprite->x > sprite2->x)
						vasemmalle = false;
					if (sprite->y < sprite2->y)
						alas = false;
					if (sprite->y > sprite2->y)
						ylos = false;
				}

				if (sprite->HasAI(AI_NUOLET_VAIKUTTAVAT)) {

					if (sprite2->HasAI(AI_NUOLI_RIGHT)) {
						sprite_a = sprite->prototype->max_speed / 3.5;
						sprite_b = 0;
					}
					else if (sprite2->HasAI(AI_NUOLI_LEFT)) {
						sprite_a = sprite->prototype->max_speed / -3.5;
						sprite_b = 0;
					}

					if (sprite2->HasAI(AI_NUOLI_UP)) {
						sprite_b = sprite->prototype->max_speed / -3.5;
						sprite_a = 0;
					}
					else if (sprite2->HasAI(AI_NUOLI_DOWN)) {
						sprite_b = sprite->prototype->max_speed / 3.5;
						sprite_a = 0;
					}
				}

				/* sprites can exchange information about a player's whereabouts */ //TODO - test this sometime
	/*			if (sprite->seen_player_x != -1 && sprite2->seen_player_x == -1)
				{
					sprite2->seen_player_x = sprite->seen_player_x + rand()%30 - rand()%30;
					sprite->seen_player_x = -1;
				} */

				// If two sprites from different teams touch each other
				if (sprite->enemy != sprite2->enemy && sprite->emosprite != sprite2) {
					if (sprite2->prototype->type != TYPE_BACKGROUND &&
						sprite->prototype->type   != TYPE_BACKGROUND &&
						sprite2->prototype->type != TYPE_TELEPORT &&
						sprite2->damage_timer == 0 &&
						sprite->damage_timer == 0 &&
						sprite2->energy > 0 &&
						sprite->energy > 0 &&
						sprite2->saatu_vahinko < 1)
					{

						if (sprite->super_mode_timer > 0 && sprite2->super_mode_timer == 0) {
							sprite2->saatu_vahinko = 500;
							sprite2->saatu_vahinko_tyyppi = DAMAGE_ALL;
						}
						if (sprite2->super_mode_timer > 0 && sprite->super_mode_timer == 0) {
							sprite->saatu_vahinko = 500;
							sprite->saatu_vahinko_tyyppi = DAMAGE_ALL;
						}
						
						//Bounce on the sprite head
						if (sprite2->b > 2 && sprite2->weight >= 0.5 &&
							sprite2->y < sprite_y && !sprite->prototype->is_wall &&
							sprite->prototype->how_destroyed != FX_DESTRUCT_EI_TUHOUDU)
						{
							if (sprite2->super_mode_timer)
								sprite->saatu_vahinko = 500;
							else
								sprite->saatu_vahinko = (int)(sprite2->weight+sprite2->b/4);
							sprite->saatu_vahinko_tyyppi = DAMAGE_DROP;
							sprite2->jump_timer = 1;
							if (sprite2->HasAI(AI_EGG2)) // Egg bounced, then crack
								sprite2->saatu_vahinko = sprite2->prototype->energy;
						}

						// If there is another sprite damaging
						if (sprite->prototype->damage > 0 && sprite2->prototype->type != TYPE_BONUS) {
							
							sprite2->saatu_vahinko        = sprite->prototype->damage;
							sprite2->saatu_vahinko_tyyppi = sprite->prototype->damage_type;
							
							if ( !(sprite2->player && sprite2->invisible_timer) ) //If sprite2 isn't a invisible player
								sprite->attack1_timer = sprite->prototype->attack1_time; //Then sprite attack??

							// The projectiles are shattered by shock
							if (sprite2->prototype->type == TYPE_PROJECTILE) {
								sprite->saatu_vahinko = 1;//sprite2->prototype->damage;
								sprite->saatu_vahinko_tyyppi = sprite2->prototype->damage_type;
							}

							if (sprite->prototype->type == TYPE_PROJECTILE) {
								sprite->saatu_vahinko = 1;//sprite2->prototype->damage;
								sprite->saatu_vahinko_tyyppi = sprite2->prototype->damage_type;
							}
						}
					}
				}

				// lis�t��n spriten painoon sit� koskettavan toisen spriten weight
				if (sprite->weight > 0)
					sprite->kytkinpaino += sprite2->prototype->weight;

			}
		}
	}

	/*****************************************************************************************/
	/* If the sprite has suffered damage                                                     */
	/*****************************************************************************************/
	if (sprite->saatu_vahinko != 0 && sprite->super_mode_timer != 0) {
		sprite->saatu_vahinko = 0;
		sprite->saatu_vahinko_tyyppi = DAMAGE_NONE;
	}

	// If it is invisible, just these damages can injury it
	if (sprite->saatu_vahinko != 0 && sprite->invisible_timer != 0 && 
		sprite->saatu_vahinko_tyyppi != DAMAGE_FIRE &&
		sprite->saatu_vahinko_tyyppi != DAMAGE_COMPRESSION &&
		sprite->saatu_vahinko_tyyppi != DAMAGE_DROP &&
		sprite->saatu_vahinko_tyyppi != DAMAGE_ALL) {
		
		sprite->saatu_vahinko = 0;
		sprite->saatu_vahinko_tyyppi = DAMAGE_NONE;
	}

	if (sprite->saatu_vahinko != 0 && sprite->energy > 0 && sprite->prototype->how_destroyed != FX_DESTRUCT_EI_TUHOUDU){
		if (sprite->prototype->immunity_type != sprite->saatu_vahinko_tyyppi || sprite->prototype->immunity_type == DAMAGE_NONE){
			sprite->energy -= sprite->saatu_vahinko;
			sprite->damage_timer = DAMAGE_TIME;

			if (sprite->saatu_vahinko_tyyppi == DAMAGE_ELECTRIC)
				sprite->damage_timer *= 6;

			Play_GameSFX(sprite->prototype->sounds[SOUND_DAMAGE], 100, (int)sprite->x, (int)sprite->y,
						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			if (sprite->prototype->how_destroyed%100 == FX_DESTRUCT_HOYHENET)
				Effect_Destruction(FX_DESTRUCT_HOYHENET, (u32)sprite->x, (u32)sprite->y);

			if (sprite->prototype->type != TYPE_PROJECTILE){
				Particles_New(PARTICLE_STAR,sprite_x,sprite_y,-1,-1,60,0.01,128);
				Particles_New(PARTICLE_STAR,sprite_x,sprite_y, 0,-1,60,0.01,128);
				Particles_New(PARTICLE_STAR,sprite_x,sprite_y, 1,-1,60,0.01,128);
			}

			if (sprite->HasAI(AI_VAIHDA_KALLOT_JOS_OSUTTU))
				Game->Change_SkullBlocks();

			if (sprite->HasAI(AI_ATTACK_1_JOS_OSUTTU)){
				sprite->attack1_timer = sprite->prototype->attack1_time;
				sprite->charging_timer = 0;
			}

			if (sprite->HasAI(AI_ATTACK_2_JOS_OSUTTU)){
				sprite->attack2_timer = sprite->prototype->attack2_time;
				sprite->charging_timer = 0;
			}

		}

		sprite->saatu_vahinko = 0;
		sprite->saatu_vahinko_tyyppi = DAMAGE_NONE;


		/*****************************************************************************************/
		/* If the sprite is destroyed                                                            */
		/*****************************************************************************************/

		if (sprite->energy < 1) {
			how_destroyed = sprite->prototype->how_destroyed;

			if (how_destroyed != FX_DESTRUCT_EI_TUHOUDU) {
				SpriteOnDeath(sprite);
			} else
				sprite->energy = 1;
		}
	}

	if (sprite->damage_timer == 0)
		sprite->saatu_vahinko_tyyppi = DAMAGE_NONE;


	/*****************************************************************************************/
	/* Revisions                                                                             */
	/*****************************************************************************************/

	/*if (&sprite == Player_Sprite && dev_mode) {

		oikealle   = true;
		vasemmalle = true;
		ylos       = true;
		alas       = true;
		printf("%f\n", sprite_b);

	}*/

	if (!oikealle)
		if (sprite_a > 0)
			sprite_a = 0;

	if (!vasemmalle)
		if (sprite_a < 0)
			sprite_a = 0;

	if (!ylos){
		if (sprite_b < 0)
			sprite_b = 0;

		if (!hyppy_maximissa)
			sprite->jump_timer = 95;//sprite->prototype->max_jump * 2;
	}

	if (!alas)
		if (sprite_b >= 0){ //If sprite is falling
			if (sprite->jump_timer > 0){
				if (sprite->jump_timer >= 90+10){
					Play_GameSFX(pump_sound,30,(int)sprite_x, (int)sprite_y,
				                  int(25050-sprite->weight*3000),true);

					//Particles_New(	PARTICLE_DUST_CLOUDS,sprite_x+rand()%5-rand()%5-10,sprite_ala+rand()%3-rand()%3,
					//			  0,-0.2,rand()%50+20,0,0);

					if (rand()%7 == 1) {
						Particles_New(PARTICLE_SMOKE,sprite_x+rand()%5-rand()%5-10,sprite_ala+rand()%3-rand()%3,
									  	   0.3,-0.1,450,0,0);
						Particles_New(PARTICLE_SMOKE,sprite_x+rand()%5-rand()%5-10,sprite_ala+rand()%3-rand()%3,
									  	   -0.3,-0.1,450,0,0);
					}

					if (sprite->weight > 1) {
						Game->vibration = 34 + int(sprite->weight * 20);
						PInput::Vibrate(500);
					}
				}

				sprite->jump_timer = 0;
			}

			sprite_b = 0;
		}

	/*****************************************************************************************/
	/* Set correct values                                                                    */
	/*****************************************************************************************/

	if (sprite_b > 4.0)
		sprite_b = 4.0;

	if (sprite_b < -4.0)
		sprite_b = -4.0;

	//Limit speed 2
	if (sprite_a > max_speed)
		sprite_a = max_speed;

	if (sprite_a < -max_speed)
		sprite_a = -max_speed;

	if (sprite->energy < 0)
		sprite->energy = 0;

	if (sprite->energy > sprite->prototype->energy)
		sprite->energy = sprite->prototype->energy;

	if (sprite->damage_timer == 0 || sprite->player == 1) {
		sprite_x += sprite_a;
		sprite_y += sprite_b;
	}

	if (sprite == Player_Sprite || sprite->energy < 1) {
		double kitka = 1.04;

		if (Game->map.weather == WEATHER_RAIN || Game->map.weather == WEATHER_RAIN_LEAVES)
			kitka = 1.03; // Slippery ground in the rain

		if (Game->map.weather == WEATHER_SNOW)
			kitka = 1.01; // And even more on snow

		if (!alas)
			sprite_a /= kitka;
		else
			sprite_a /= 1.03;//1.02//1.05

		sprite_b /= 1.25;
	}

	sprite->x = sprite_x;
	sprite->y = sprite_y;
	sprite->a = sprite_a;
	sprite->b = sprite_b;

	sprite->oikealle = oikealle;
	sprite->vasemmalle = vasemmalle;
	sprite->alas = alas;
	sprite->ylos = ylos;

	/*
	sprite->weight = sprite->prototype->weight;

	if (sprite->energy < 1 && sprite->weight == 0)
		sprite->weight = 1;*/

	if (sprite->jump_timer < 0)
		sprite->alas = false;

	//sprite->crouched   = false;

	/*****************************************************************************************/
	/* AI                                                                                    */
	/*****************************************************************************************/

	//TODO run sprite lua script
	// or maybe Python, JS or something different?
	// Forgive me, but personally I don't like lua.
	
	if (sprite->player == 0) {

		for(const int& sprite_ai: sprite->prototype->AI_v){
			switch (sprite_ai) {
				case AI_KANA:						sprite->AI_Kana();
													break;
				case AI_LITTLE_CHICKEN:					sprite->AI_Kana();
													break;
				case AI_SAMMAKKO1:					sprite->AI_Sammakko1();
													break;
				case AI_SAMMAKKO2:					sprite->AI_Sammakko2();
													break;
				case AI_BONUS:						sprite->AI_Bonus();
													break;
				case AI_EGG:						sprite->AI_Egg();
													break;
				case AI_EGG2:						sprite->AI_Egg2();
													break;
				case AI_AMMUS:						sprite->AI_Ammus();
													break;
				case AI_JUMPER:					sprite->AI_Jumper();
													break;
				case AI_BASIC:						sprite->AI_Basic();
													break;
				case AI_NONSTOP:					sprite->AI_NonStop();
													break;
				case AI_TURNING_HORIZONTALLY:		sprite->AI_Kaantyy_Esteesta_Hori();
													break;
				case AI_KAANTYY_ESTEESTA_VERT:		sprite->AI_Kaantyy_Esteesta_Vert();
													break;
				case AI_LOOK_FOR_CLIFFS:				sprite->AI_Varoo_Kuoppaa();
													break;
				case AI_RANDOM_CHANGE_DIRECTION_H:	sprite->AI_Random_Suunnanvaihto_Hori();
													break;
				case AI_RANDOM_KAANTYMINEN:			sprite->AI_Random_Kaantyminen();
													break;
				case AI_RANDOM_JUMP:				sprite->AI_Random_Hyppy();
													break;
				case AI_FOLLOW_PLAYER:			if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Seuraa_Pelaajaa(*Player_Sprite);
													break;
				case AI_FOLLOW_PLAYER_IF_IN_FRONT:	if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Seuraa_Pelaajaa_Jos_Nakee(*Player_Sprite);
													break;
				case AI_FOLLOW_PLAYER_VERT_HORI:	if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Seuraa_Pelaajaa_Vert_Hori(*Player_Sprite);
													break;
				case AI_FOLLOW_PLAYER_IF_IN_FRONT_VERT_HORI:
													if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Seuraa_Pelaajaa_Jos_Nakee_Vert_Hori(*Player_Sprite);
													break;
				case AI_PAKENEE_PELAAJAA_JOS_NAKEE:	if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Pakenee_Pelaajaa_Jos_Nakee(*Player_Sprite);
													break;
				case AI_POMMI:						sprite->AI_Pommi();
													break;
				case AI_ATTACK_1_JOS_OSUTTU:		sprite->AI_Attack_1_Jos_Osuttu();
													break;
				case AI_ATTACK_2_JOS_OSUTTU:		sprite->AI_Attack_2_Jos_Osuttu();
													break;
				case AI_ATTACK_1_NONSTOP:			sprite->AI_Attack_1_Nonstop();
													break;
				case AI_ATTACK_2_NONSTOP:			sprite->AI_Attack_2_Nonstop();
													break;
				case AI_ATTACK_1_IF_PLAYER_IN_FRONT:
													if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Attack_1_if_Player_in_Front(*Player_Sprite);
													break;
				case AI_ATTACK_2_IF_PLAYER_IN_FRONT:
													if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Attack_2_if_Player_in_Front(*Player_Sprite);
													break;
				case AI_ATTACK_1_IF_PLAYER_BELLOW:
													if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Attack_1_if_Player_Bellow(*Player_Sprite);
													break;
				case AI_HYPPY_JOS_PELAAJA_YLAPUOLELLA:
													if (Player_Sprite->invisible_timer == 0)
														sprite->AI_Hyppy_Jos_Pelaaja_Ylapuolella(*Player_Sprite);
													break;
				case AI_DAMAGED_BY_WATER:		sprite->AI_Damaged_by_Water();
													break;
				case AI_KILL_EVERYONE:				sprite->AI_Kill_Everyone();
													break;
				case AI_KITKA_VAIKUTTAA:			sprite->AI_Kitka_Vaikuttaa();
													break;
				case AI_PIILOUTUU:					sprite->AI_Piiloutuu();
													break;
				case AI_PALAA_ALKUUN_X:				sprite->AI_Palaa_Alkuun_X();
													break;
				case AI_PALAA_ALKUUN_Y:				sprite->AI_Palaa_Alkuun_Y();
													break;
				case AI_LIIKKUU_X_COS:				sprite->AI_Liikkuu_X(cos_table(degree));
													break;
				case AI_LIIKKUU_Y_COS:				sprite->AI_Liikkuu_Y(cos_table(degree));
													break;
				case AI_LIIKKUU_X_SIN:				sprite->AI_Liikkuu_X(sin_table(degree));
													break;
				case AI_LIIKKUU_Y_SIN:				sprite->AI_Liikkuu_Y(sin_table(degree));
													break;
				case AI_LIIKKUU_X_COS_NOPEA:		sprite->AI_Liikkuu_X(cos_table(degree*2));
													break;
				case AI_LIIKKUU_Y_SIN_NOPEA:		sprite->AI_Liikkuu_Y(sin_table(degree*2));
													break;
				case AI_LIIKKUU_X_COS_HIDAS:		sprite->AI_Liikkuu_X(cos_table(degree/2));
													break;
				case AI_LIIKKUU_Y_SIN_HIDAS:		sprite->AI_Liikkuu_Y(sin_table(degree/2));
													break;
				case AI_LIIKKUU_Y_SIN_VAPAA:		sprite->AI_Liikkuu_Y(sin_table(sprite->action_timer/2));
													break;
				case AI_CHANGE_WHEN_ENERGY_UNDER_2:	if (sprite->prototype->transformation != nullptr)
														sprite->AI_Change_When_Energy_Under_2(sprite->prototype->transformation);
													break;
				case AI_CHANGE_WHEN_ENERGY_OVER_1:	if (sprite->prototype->transformation != nullptr)
														if (sprite->AI_Change_When_Energy_Over_1(sprite->prototype->transformation)==1)
															Effect_Destruction(FX_DESTRUCT_SAVU_HARMAA, (u32)sprite->x, (u32)sprite->y);
													break;
				case AI_MUUTOS_AJASTIN:				if (sprite->prototype->transformation != nullptr) {
														sprite->AI_Muutos_Ajastin(sprite->prototype->transformation);
													}
													break;
				case AI_MUUTOS_JOS_OSUTTU:			if (sprite->prototype->transformation != nullptr) {
														sprite->AI_Muutos_Jos_Osuttu(sprite->prototype->transformation);
													}
													break;
				case AI_TELEPORT:					if (sprite->AI_Teleportti(Sprites_List, *Player_Sprite)==1)
													{

														Game->camera_x = (int)Player_Sprite->x;
														Game->camera_y = (int)Player_Sprite->y;
														Game->dcamera_x = Game->camera_x-screen_width/2;
														Game->dcamera_y = Game->camera_y-screen_height/2;
														Fade_in(FADE_NORMAL);
														if (sprite->prototype->sounds[SOUND_ATTACK2] != -1)
															Play_MenuSFX(sprite->prototype->sounds[SOUND_ATTACK2], 100);
															//Play_GameSFX(, 100, Game->camera_x, Game->camera_y, SOUND_SAMPLERATE, false);


													}
													break;
				case AI_KIIPEILIJA:					sprite->AI_Kiipeilija();
													break;
				case AI_KIIPEILIJA2:				sprite->AI_Kiipeilija2();
													break;
				case AI_TUHOUTUU_JOS_EMO_TUHOUTUU:	sprite->AI_Tuhoutuu_Jos_Emo_Tuhoutuu();
													break;

				case AI_TIPPUU_TARINASTA:			sprite->AI_Tippuu_Tarinasta(Game->vibration + Game->button_vibration);
													break;
				case AI_LIIKKUU_DOWN_JOS_KYTKIN1_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button1,0,1);
													break;
				case AI_LIIKKUU_UP_JOS_KYTKIN1_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button1,0,-1);
													break;
				case AI_LIIKKUU_LEFT_JOS_KYTKIN1_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button1,-1,0);
													break;
				case AI_LIIKKUU_RIGHT_JOS_KYTKIN1_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button1,1,0);
													break;
				case AI_LIIKKUU_DOWN_JOS_KYTKIN2_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button2,0,1);
													break;
				case AI_LIIKKUU_UP_JOS_KYTKIN2_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button2,0,-1);
													break;
				case AI_LIIKKUU_LEFT_JOS_KYTKIN2_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button2,-1,0);
													break;
				case AI_LIIKKUU_RIGHT_JOS_KYTKIN2_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button2,1,0);
													break;
				case AI_LIIKKUU_DOWN_JOS_KYTKIN3_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button3,0,1);
													break;
				case AI_LIIKKUU_UP_JOS_KYTKIN3_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button3,0,-1);
													break;
				case AI_LIIKKUU_LEFT_JOS_KYTKIN3_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button3,-1,0);
													break;
				case AI_LIIKKUU_RIGHT_JOS_KYTKIN3_PAINETTU: sprite->AI_Liikkuu_Jos_Kytkin_Painettu(Game->button3,1,0);
													break;
				case AI_TIPPUU_JOS_KYTKIN1_PAINETTU: sprite->AI_Tippuu_Jos_Kytkin_Painettu(Game->button1);
													break;
				case AI_TIPPUU_JOS_KYTKIN2_PAINETTU: sprite->AI_Tippuu_Jos_Kytkin_Painettu(Game->button2);
													break;
				case AI_TIPPUU_JOS_KYTKIN3_PAINETTU: sprite->AI_Tippuu_Jos_Kytkin_Painettu(Game->button3);
													break;
				case AI_RANDOM_LIIKAHDUS_VERT_HORI:	sprite->AI_Random_Liikahdus_Vert_Hori();
													break;
				case AI_KAANTYY_JOS_OSUTTU:			sprite->AI_Kaantyy_Jos_Osuttu();
													break;
				case AI_EVIL_ONE:					if (sprite->energy < 1) 
													{
														PSound::set_musicvolume(0);
														Game->music_stopped = true;
													}
													break;
				
				case AI_DESTRUCTED_NEXT_TO_PLAYER:	sprite->AI_Destructed_Next_To_Player(*Player_Sprite);
													break;

				default:

				if (sprite_ai >= AI_INFOS_BEGIN && sprite_ai <= AI_INFOS_END)
					if (sprite->AI_Info(*Player_Sprite)) {

						int info = sprite_ai - AI_INFOS_BEGIN + 1;
						
						std::string sinfo = "info";
						if (info < 10) sinfo += '0';
						sinfo += std::to_string(info);

						int index = Episode->infos.Search_Id(sinfo.c_str());
						if (index != -1)
							Game->Show_Info(Episode->infos.Get_Text(index));
						else
							Game->Show_Info(tekstit->Get_Text(PK_txt.infos[info]));

					}
			}



		}
	}

	//if (kaiku == 1 && sprite->prototype->prototype == TYPE_PROJECTILE && sprite->prototype->damage_type == DAMAGE_NOISE &&
	//	sprite->prototype->sounds[SOUND_ATTACK1] > -1)
	//	Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK1],20, (int)sprite_x, (int)sprite_y,
	//				  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);


	/*****************************************************************************************/
	/* Set game area to sprite                                                               */
	/*****************************************************************************************/

	if (sprite->x < 0)
		sprite->x = 0;

	if (sprite->y < -sprite_korkeus)
		sprite->y = -sprite_korkeus;

	if (sprite->x > PK2MAP_MAP_WIDTH*32)
		sprite->x = PK2MAP_MAP_WIDTH*32;

	// If the sprite falls under the lower edge of the map
	if (sprite->y > PK2MAP_MAP_HEIGHT*32 + sprite_korkeus) {

		sprite->y = PK2MAP_MAP_HEIGHT*32 + sprite_korkeus;
		sprite->energy = 0;
		sprite->removed = true;

		if (sprite->kytkinpaino >= 1)
			Game->vibration = 50;
	}

	if (sprite->a > max_speed)
		sprite->a = max_speed;

	if (sprite->a < -max_speed)
		sprite->a = -max_speed;


	/*****************************************************************************************/
	/* Attacks 1 and 2                                                                       */
	/*****************************************************************************************/

	// If the sprite is ready and isn't crouching
	if (sprite->charging_timer == 0 && !sprite->crouched) {
		// the attack has just started
		if (sprite->attack1_timer == sprite->prototype->attack1_time) {
			// provides recovery time, after which the sprite can attack again
			sprite->charging_timer = sprite->prototype->charge_time;
			if(sprite->charging_timer == 0) sprite->charging_timer = 5;
			// if you don't have your own charging time ...
			if (sprite->ammo1 != nullptr && sprite->prototype->charge_time == 0)
			// ... and the projectile has, take charge_time from the projectile
				if (sprite->ammo1->projectile_charge_time > 0)
					sprite->charging_timer = sprite->ammo1->projectile_charge_time;

			// attack sound
			Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK1],100, (int)sprite_x, (int)sprite_y,
						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			if (sprite->ammo1 != nullptr) {
				Sprites_add_ammo(sprite->ammo1,sprite_x, sprite_y, sprite);

		//		if (Level_Prototypes_List[sprite->ammo1].sounds[SOUND_ATTACK1] > -1)
		//			Play_GameSFX(Level_Prototypes_List[sprite->ammo1].sounds[SOUND_ATTACK1],100, (int)sprite_x, (int)sprite_y,
		//						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);
			}
		}

		// Same as attack 1
		if (sprite->attack2_timer == sprite->prototype->attack2_time) {
			sprite->charging_timer = sprite->prototype->charge_time;
			if(sprite->charging_timer == 0) sprite->charging_timer = 5;
			if (sprite->ammo2 != nullptr && sprite->prototype->charge_time == 0)
				if (sprite->ammo2->projectile_charge_time > 0)
					sprite->charging_timer = sprite->ammo2->projectile_charge_time;

			Play_GameSFX(sprite->prototype->sounds[SOUND_ATTACK2],100,(int)sprite_x, (int)sprite_y,
						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			if (sprite->ammo2 != nullptr) {
				Sprites_add_ammo(sprite->ammo2,sprite_x, sprite_y, sprite);

		//		if (Level_Prototypes_List[sprite->ammo2].sounds[SOUND_ATTACK1] > -1)
		//			Play_GameSFX(Level_Prototypes_List[sprite->ammo2].sounds[SOUND_ATTACK1],100, (int)sprite_x, (int)sprite_y,
		//						  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

			}
		}
	}

	// Random sounds
	if (sprite->prototype->sounds[SOUND_RANDOM] != -1 && rand()%200 == 1 && sprite->energy > 0)
		Play_GameSFX(sprite->prototype->sounds[SOUND_RANDOM],80,(int)sprite_x, (int)sprite_y,
					  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

	return 0;

}

int UpdateBonusSprite(SpriteClass* sprite){

	sprite_x = sprite->x;
	sprite_y = sprite->y;
	sprite_a = sprite->a;
	sprite_b = sprite->b;

	sprite_leveys  = sprite->prototype->width;
	sprite_korkeus = sprite->prototype->height;

	sprite_vasen = sprite_x - sprite_leveys  / 2;
	sprite_oikea = sprite_x + sprite_leveys  / 2;
	sprite_yla   = sprite_y - sprite_korkeus / 2;
	sprite_ala   = sprite_y + sprite_korkeus / 2;

	oikealle	= true,
	vasemmalle	= true,
	ylos		= true,
	alas		= true;

	in_water = sprite->in_water;

	max_speed = sprite->prototype->max_speed;

	if (sprite->damage_timer > 0)
		sprite->damage_timer--;

	if (sprite->charging_timer > 0)
		sprite->charging_timer--;

	if (sprite->mutation_timer > 0)	// aika muutokseen
		sprite->mutation_timer --;

	// Hyppyyn liittyv�t seikat

	if (Game->button_vibration + Game->vibration > 0 && sprite->jump_timer == 0)
		sprite->jump_timer = sprite->prototype->max_jump / 2;

	if (sprite->jump_timer > 0 && sprite->jump_timer < sprite->prototype->max_jump)
	{
		sprite->jump_timer ++;
		sprite_b = (sprite->prototype->max_jump - sprite->jump_timer)/-4.0;//-2
	}

	if (sprite_b > 0)
		sprite->jump_timer = sprite->prototype->max_jump;



	if (sprite->weight != 0)	// jos bonuksella on weight, tutkitaan ymp�rist�
	{
		// o
		//
		// |  Gravity
		// V

		sprite_b += sprite->weight + sprite_b/1.25;

		if (sprite->in_water)
		{
			if (sprite_b > 0)
				sprite_b /= 2.0;

			if (rand()%80 == 1)
				Particles_New(PARTICLE_SPARK,sprite_x-4,sprite_y,0,-0.5-rand()%2,rand()%30+30,0,32);
		}

		sprite->in_water = false;

		sprite->kytkinpaino = sprite->weight;

		/* TOISET SPRITET */

		PK2BLOCK spritepalikka; 

		for (SpriteClass* sprite2 : Sprites_List) {
			if (sprite2 != sprite && !sprite2->removed) {
				if (sprite2->prototype->is_wall && sprite->prototype->makes_sounds) {
					if (sprite_x-sprite_leveys/2 +sprite_a <= sprite2->x + sprite2->prototype->width /2 &&
						sprite_x+sprite_leveys/2 +sprite_a >= sprite2->x - sprite2->prototype->width /2 &&
						sprite_y-sprite_korkeus/2+sprite_b <= sprite2->y + sprite2->prototype->height/2 &&
						sprite_y+sprite_korkeus/2+sprite_b >= sprite2->y - sprite2->prototype->height/2)
					{
						spritepalikka.koodi = 0;
						spritepalikka.ala   = (int)sprite2->y + sprite2->prototype->height/2;
						spritepalikka.oikea = (int)sprite2->x + sprite2->prototype->width/2;
						spritepalikka.vasen = (int)sprite2->x - sprite2->prototype->width/2;
						spritepalikka.yla   = (int)sprite2->y - sprite2->prototype->height/2;

						spritepalikka.alas       = BLOCK_WALL;
						spritepalikka.ylos       = BLOCK_WALL;
						spritepalikka.oikealle   = BLOCK_WALL;
						spritepalikka.vasemmalle = BLOCK_WALL;

						if (!sprite2->prototype->is_wall_down)
							spritepalikka.alas		 = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_up)
							spritepalikka.ylos		 = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_right)
							spritepalikka.oikealle   = BLOCK_BACKGROUND;
						if (!sprite2->prototype->is_wall_left)
							spritepalikka.vasemmalle = BLOCK_BACKGROUND;


						spritepalikka.water  = false;

						spritepalikka.tausta = false;

						// Bonus accepts just walls?
						/*spritepalikka.oikealle   = BLOCK_WALL;
						spritepalikka.vasemmalle = BLOCK_WALL;
						spritepalikka.ylos       = BLOCK_WALL;
						spritepalikka.alas       = BLOCK_WALL;*/

						Check_SpriteBlock(sprite, spritepalikka); //Colision bonus and sprite block
					}
				}

				if (sprite_x < sprite2->x + sprite2->prototype->width/2 &&
					sprite_x > sprite2->x - sprite2->prototype->width/2 &&
					sprite_y < sprite2->y + sprite2->prototype->height/2 &&
					sprite_y > sprite2->y - sprite2->prototype->height/2 &&
					sprite->damage_timer == 0)
				{
					if (sprite2->prototype->type != TYPE_BONUS &&
						!(sprite2 == Player_Sprite && sprite->prototype->how_destroyed != FX_DESTRUCT_EI_TUHOUDU))
						sprite_a += sprite2->a*(rand()%4);

					// lis�t��n spriten painoon sit� koskettavan toisen spriten weight
					sprite->kytkinpaino += sprite2->prototype->weight;

					// samanmerkkiset spritet vaihtavat suuntaa t�rm�tess��n
					if (sprite->prototype == sprite2->prototype &&
						sprite2->energy > 0)
					{
						if (sprite->x < sprite2->x) {
							sprite2->a += sprite->a / 3.0;
							oikealle = false;
						}
						if (sprite->x > sprite2->x) {
							sprite2->a += sprite->a / 3.0;
							vasemmalle = false;
						}
					}

				}
			}
		}

		// Tarkistetaan ettei menn� mihink��n suuntaan liian kovaa.

		if (sprite_b > 4)
			sprite_b = 4;

		if (sprite_b < -4)
			sprite_b = -4;

		if (sprite_a > 3)
			sprite_a = 3;

		if (sprite_a < -3)
			sprite_a = -3;

		// Lasketaan

		if (sprite->prototype->makes_sounds)
		{

			int palikat_x_lkm = (int)((sprite_leveys) /32)+4;
			int palikat_y_lkm = (int)((sprite_korkeus)/32)+4;

			int map_vasen = (int)(sprite_vasen)/32;
			int map_yla   = (int)(sprite_yla)/32;

			// Tutkitaan t�rm��k� palikkaan

			for (int y = 0; y < palikat_y_lkm; y++)
				for (int x = 0; x < palikat_x_lkm; x++){
					PK2BLOCK block = Block_Get(map_vasen+x-1,map_yla+y-1);
					Check_MapBlock(sprite, block);
				}

		}

		if (in_water != sprite->in_water) {
			Effect_Splash((int)sprite_x,(int)sprite_y,32);
			Play_GameSFX(splash_sound, 100, (int)sprite_x, (int)sprite_y, SOUND_SAMPLERATE, true);
		}


		if (!oikealle)
		{
			if (sprite_a > 0)
				sprite_a = -sprite_a/1.5;
		}

		if (!vasemmalle)
		{
			if (sprite_a < 0)
				sprite_a = -sprite_a/1.5;
		}

		if (!ylos)
		{
			if (sprite_b < 0)
				sprite_b = 0;

			sprite->jump_timer = sprite->prototype->max_jump;
		}

		if (!alas)
		{
			if (sprite_b >= 0)
			{
				if (sprite->jump_timer > 0)
				{
					sprite->jump_timer = 0;
				//	if (/*sprite_b == 4*/!maassa)
				//		Play_GameSFX(pump_sound,20,(int)sprite_x, (int)sprite_y,
				//				      int(25050-sprite->prototype->weight*4000),true);
				}

				if (sprite_b > 2)
					sprite_b = -sprite_b/(3+rand()%2);
				else
					sprite_b = 0;
			}
			//sprite_a /= kitka;
			sprite_a /= 1.07;
		}
		else
		{
			sprite_a /= 1.02;
		}

		sprite_b /= 1.5;

		if (sprite_b > 4)
			sprite_b = 4;

		if (sprite_b < -4)
			sprite_b = -4;

		if (sprite_a > 4)
			sprite_a = 4;

		if (sprite_a < -4)
			sprite_a = -4;

		sprite_x += sprite_a;
		sprite_y += sprite_b;

		sprite->x = sprite_x;
		sprite->y = sprite_y;
		sprite->a = sprite_a;
		sprite->b = sprite_b;

		sprite->oikealle = oikealle;
		sprite->vasemmalle = vasemmalle;
		sprite->alas = alas;
		sprite->ylos = ylos;
	}
	else	// jos spriten weight on nolla, tehd��n spritest� "kelluva"
	{
		sprite->y = sprite->orig_y + cos_table(degree+(sprite->orig_x+sprite->orig_y)) / 3.0;
		sprite_y = sprite->y;
	}

	sprite->weight = sprite->initial_weight;

	int how_destroyed;

	// Test if player touches bonus
	if (sprite_x < Player_Sprite->x + Player_Sprite->prototype->width/2 &&
		sprite_x > Player_Sprite->x - Player_Sprite->prototype->width/2 &&
		sprite_y < Player_Sprite->y + Player_Sprite->prototype->height/2 &&
		sprite_y > Player_Sprite->y - Player_Sprite->prototype->height/2 &&
		sprite->damage_timer == 0)
	{
		if (sprite->energy > 0 && Player_Sprite->energy > 0)
		{
			if (sprite->prototype->score != 0) {
				if(sprite->prototype->big_apple){
					Game->apples_got++;
				}

				Game->score_increment += sprite->prototype->score;
				
				if (!sprite->HasAI(AI_BONUS_AIKA)) {
					int font = sprite->prototype->score >= 50? fontti2 : fontti1;
					Fadetext_New(font, std::to_string(sprite->prototype->score), (int)sprite->x-8,(int)sprite->y-8,100);
				}

			}

			if (sprite->HasAI(AI_BONUS_AIKA)) {
				
				int increase_time = sprite->prototype->charge_time * TIME_FPS;
				Game->timeout += increase_time;

				float shown_time = float(increase_time) / 60;
				int minutes = int(shown_time / 60);
				int seconds = int(shown_time) % 60;

				std::ostringstream os;
				os<<minutes<<":";
				if(seconds<10){
					os<<"0";
				}
				os<<seconds;
				
				Fadetext_New(fontti1,os.str(),(int)sprite->x-15,(int)sprite->y-8,100);
			}

			if (sprite->HasAI(AI_BONUS_NAKYMATTOMYYS))
				Player_Sprite->invisible_timer = sprite->prototype->charge_time;

			if (sprite->HasAI(AI_BONUS_SUPERMODE)) {
				Player_Sprite->super_mode_timer = sprite->prototype->charge_time;
				PSound::play_overlay_music();
			}

			//Game->map.spritet[(int)(sprite->orig_x/32) + (int)(sprite->orig_y/32)*PK2MAP_MAP_WIDTH] = 255;

			if (sprite->prototype->how_destroyed != FX_DESTRUCT_EI_TUHOUDU)
				Player_Sprite->energy -= sprite->prototype->damage;

			how_destroyed = sprite->prototype->how_destroyed;

			if (how_destroyed != FX_DESTRUCT_EI_TUHOUDU)
			{
				if (how_destroyed >= FX_DESTRUCT_ANIMAATIO)
					how_destroyed -= FX_DESTRUCT_ANIMAATIO;
				else
				{
					if (sprite->prototype->can_open_locks)
					{
						Game->keys--;

						if (Game->keys < 1)
							Game->Open_Locks();
					}

					sprite->removed = true;
				}

				if (sprite->HasAI(AI_UUSI_JOS_TUHOUTUU)) {
					double ax, ay;
					ax = sprite->orig_x;//-sprite->prototype->width;
					ay = sprite->orig_y-sprite->prototype->height/2.0;
					Sprites_add(sprite->prototype, 0, ax-17, ay, sprite, false);
					for (int r=1;r<6;r++)
						Particles_New(PARTICLE_SPARK,ax+rand()%10-rand()%10, ay+rand()%10-rand()%10,0,0,rand()%100,0.1,32);

				}

				if (sprite->prototype->bonus != nullptr)
					if (Gifts_Add(sprite->prototype->bonus))
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newitem));

				if (sprite->prototype->transformation != nullptr)
				{
					if (sprite->prototype->transformation->first_ai() != AI_BONUS)
					{
						Player_Sprite->prototype = sprite->prototype->transformation;
						Player_Sprite->ammo1 = Player_Sprite->prototype->ammo1;
						Player_Sprite->ammo2 = Player_Sprite->prototype->ammo2;
						Player_Sprite->initial_weight = Player_Sprite->prototype->weight;
						Player_Sprite->y -= Player_Sprite->prototype->height/2;
						
						int infotext = Episode->infos.Search_Id("pekka transformed");
						if (infotext != -1)
							Game->Show_Info(Episode->infos.Get_Text(infotext));
						//Game->Show_Info("pekka has been transformed!");
					}
				}

				if (sprite->prototype->ammo1 != nullptr) {
					Player_Sprite->ammo1 = sprite->prototype->ammo1;

					int infotext = Episode->infos.Search_Id("new egg attack");
					if (infotext != -1)
						Game->Show_Info(Episode->infos.Get_Text(infotext));
					else
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newegg));
				}

				if (sprite->prototype->ammo2 != nullptr) {
					Player_Sprite->ammo2 = sprite->prototype->ammo2;

					int infotext = Episode->infos.Search_Id("new doodle attack");
					if (infotext != -1)
						Game->Show_Info(Episode->infos.Get_Text(infotext));
					else
						Game->Show_Info(tekstit->Get_Text(PK_txt.game_newdoodle));
				}

				Play_GameSFX(sprite->prototype->sounds[SOUND_DESTRUCTION],100, (int)sprite->x, (int)sprite->y,
							  sprite->prototype->sound_frequency, sprite->prototype->random_sound_frequency);

				Effect_Destruction(how_destroyed, (u32)sprite_x, (u32)sprite_y);
			}
		}
	}
	for(const int& sprite_ai: sprite->prototype->AI_v){

		switch (sprite_ai) {
		
		case AI_BONUS:				sprite->AI_Bonus(); break;

		case AI_BASIC:				sprite->AI_Basic(); break;

		case AI_MUUTOS_AJASTIN:		if (sprite->prototype->transformation != nullptr)
										sprite->AI_Muutos_Ajastin(sprite->prototype->transformation);
									break;

		case AI_TIPPUU_TARINASTA:	sprite->AI_Tippuu_Tarinasta(Game->vibration + Game->button_vibration);
									break;

		default:					break;
		
		}
	}

	/* The energy doesn't matter that the player is a bonus item */
	if (sprite->player != 0)
		sprite->energy = 0;

	return 0;
}

