#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "SmashCharacter.h"
#include "Constants.h"
#include "GameLibrary\InputHandler.h"
#include "SmashWorld.h"

SmashCharacter::SmashCharacter(int player_idx, Json::Value playerBestiaryData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	BoulderCreature::BoulderCreature(player_idx, window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_PLAYER_CHARACTER);
	name = "Player";
	player_index = player_idx;
	is_interactable = false;

	baseMaxHitPoints = playerBestiaryData["DictOfUnits"]["Player"]["HitPoints"].asInt();
	can_take_input = true;

	characterLevel = 1;
	//weaponLevel = 1;
	characterLevelForDisplay = 1;
	//weaponLevelForDisplay = 1;
	characterExperienceTowardsNextLevel = 0;
	//weaponExperienceTowardsNextLevel = 0;
	characterExperienceTowardsNextLevelForAnimatedBar = 0;
	//weaponExperienceTowardsNextLevelForAnimatedBar = 0;

	hit_points = max_hit_points = GetMaximumHitPointsFromLevel(characterLevel);

	speed = 5.0f;
	running_speed_multiplier = 2.0f;
	jump_power = 10.0f;
	attacksAreInterruptible = true;
	teleportedSinceLastLanding = false;
	rightStickWasCentered = true;

	leftStickInputs = sf::Vector2f(0.0f, 0.0f);

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setPosition(position);
	rectangle_shape = shape;
	player_color = sf::Color::Cyan;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);

	centerBoxShape.SetAsBox(dimensions.x / 4.0f - 0.01f, 0.5f);
	topCircleShape.m_p.Set(0, -0.5f); //position, relative to body position
	botCircleShape.m_p.Set(0, 0.5f); //position, relative to body position
	topCircleShape.m_radius = dimensions.x / 4.0f;
	botCircleShape.m_radius = dimensions.x / 4.0f;
	centerBoxFixtureDef.shape = &centerBoxShape;
	centerBoxFixtureDef.density = 1.0f;
	centerBoxFixtureDef.friction = 0.0f;
	centerBoxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	topCircleFixtureDef.shape = &topCircleShape;
	topCircleFixtureDef.density = 1.0f;
	topCircleFixtureDef.friction = 0.0f;
	topCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	botCircleFixtureDef.shape = &botCircleShape;
	botCircleFixtureDef.density = 1.0f;
	botCircleFixtureDef.friction = 1.0f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->BOT_CIRCLE;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);

	halfBodyHeight = ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y + botCircleShape.m_radius + topCircleShape.m_radius;

	hurt_box = new HurtBox(body, player_index, playerBestiaryData["DictOfUnits"]["Player"]["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]);

	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(dimensions.x / 8.0f, 0.05f, b2Vec2(0.0f, 0.6f), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	numberOfAttacks = (int)Attack::MOVES_COUNT;// (int)playerBestiaryData["AttackingAnimations"].size();
	attacks.push_back(new Attack(body, player_index, Attack::JAB, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::JAB]));
	attacks.push_back(new Attack(body, player_index, Attack::UP_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::UP_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::DOWN_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::FORWARD_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::UP_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::UP_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::DOWN_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::FORWARD_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::BACK_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::BACK_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::NEUTRAL_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::NEUTRAL_AIR]));

	hit_stun_timer = new StatusTimer(1);
	attack_input_buffer = new StatusTimer(8);
	jumpInputBuffer = new StatusTimer(6);
	jumpAfterWalkingOffLedgeBuffer = new StatusTimer(4);
	jumpStartUpTimer = new StatusTimer(4);

	weapon = new Weapon(window, position, sf::Vector2f(0.25f, 0.25f), true, player_index, body);// , this);
	
	healthBarRect = new sf::RectangleShape(sf::Vector2f(max_hit_points, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(max_hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);

	UpdateCharacterExperienceBar();
	//UpdateWeaponExperienceBar();

	sprite_scale = 1.0f;

	LoadAllAnimations("Player", playerBestiaryData);

	attacks_size = (int)attacks.size();

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	is_hittable = true;

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 0;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	ringbearerFont.loadFromFile("Images/RingbearerFont.ttf");

	rageLevel = 0;
	angerTowardsNextRageLevel = 0;
	angerNeededForNextRageLevel = 1000;
	//numberOfRunesYouCanActivate = 0;

	tierRageColors.push_back(sf::Color::Green);
	tierRageColors.push_back(sf::Color::Yellow);
	tierRageColors.push_back(sf::Color(255, 128, 0, 255));
	tierRageColors.push_back(sf::Color::Red);

	rageLevelText = new sf::Text(to_string(rageLevel), ringbearerFont, 25);
	rageLevelText->setFillColor(sf::Color::White);
	rageLevelText->setOutlineColor(sf::Color::Black);
	rageLevelText->setOutlineThickness(1.5f);
	rageLevelText->setPosition(5.0f, 60.0f);

	rageLevelProgressBarRect = new sf::RectangleShape(sf::Vector2f(0.0f, 15.0f));
	rageLevelProgressBarRect->setPosition(25.0f, 70.0f);
	rageLevelProgressBarRect->setFillColor(tierRageColors[0]);

	rageLevelBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(200.0f, 15.0f));
	rageLevelBarBackgroundRect->setPosition(25.0f, 70.0f);
	rageLevelBarBackgroundRect->setFillColor(sf::Color::White);
	rageLevelBarBackgroundRect->setOutlineThickness(5.0f);
	rageLevelBarBackgroundRect->setOutlineColor(sf::Color::Black);

	sf::Vector2f viewport_dimensions = Singleton<SmashWorld>().Get()->GetCamera()->viewport_dimensions;

	DpadTexture = Singleton<AssetManager>().Get()->GetTexture("Images/Runes/Dpad.png");
	DpadSprite = new sf::Sprite(*DpadTexture);
	DpadSprite->setPosition(viewport_dimensions.x - 150.0f, viewport_dimensions.y - 150.0f);

	DamageRune = new Rune("Damage", "Increases damage.", "Images/Runes/DamageRune.png");
	LifestealRune = new Rune("Lifesteal", "Restores health when hitting enemies", "Images/Runes/LifestealRune.png");
	SuperJumpRune = new Rune("Super Jump", "Increases jumping power", "Images/Runes/SuperJumpRune.png");
	DefenseRune = new Rune("Defense", "Reduces the amount of damage taken", "Images/Runes/ShieldRune.png");
	BerserkerRune = new Rune("Berserker", "More damage at low health, less damage \nat high health", "Images/Runes/BerserkerRune.png");

	MasterRunesList.push_back(DamageRune);
	MasterRunesList.push_back(LifestealRune);
	MasterRunesList.push_back(SuperJumpRune);
	MasterRunesList.push_back(DefenseRune);
	MasterRunesList.push_back(BerserkerRune);

	//DpadLeftRune = DamageRune;
	//DpadLeftRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 200.0f, viewport_dimensions.y - 125.0f);
	//DpadLeftRune->UiSprite->setPosition(DpadLeftRune->UiPosition);
	//DpadLeftRune->UiSprite->setScale(rune_scale);
	//DpadLeftRune->Equipped = true;
	//
	//DpadUpRune = LifestealRune;
	//DpadUpRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 125.0f, viewport_dimensions.y - 200.0f);
	//DpadUpRune->UiSprite->setPosition(DpadUpRune->UiPosition);
	//DpadUpRune->UiSprite->setScale(rune_scale);
	//
	//DpadRightRune = SuperJumpRune;
	//DpadRightRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 50.0f, viewport_dimensions.y - 125.0f);
	//DpadRightRune->UiSprite->setPosition(DpadRightRune->UiPosition);
	//DpadRightRune->UiSprite->setScale(rune_scale);

	rageTierAuraAnimations.push_back(new SpriteAnimation(render_window, "Units/Player/RageTier1Sheet.png", 64, 64, 8, 4, 2, 1.0f, sf::Color::White, true));
	rageTierAuraAnimations[0]->Play();
	rageTierAuraAnimations.push_back(new SpriteAnimation(render_window, "Units/Player/RageTier2Sheet.png", 64, 64, 6, 4, 2, 1.0f, sf::Color::White, true));
	rageTierAuraAnimations[1]->Play();
	rageTierAuraAnimations.push_back(new SpriteAnimation(render_window, "Units/Player/RageTier3Sheet.png", 64, 64, 4, 2, 2, 1.0f, sf::Color::White, true));
	rageTierAuraAnimations[2]->Play();

	RageAscensionSoundBuffer.loadFromFile("Sound/DbzSoundEffectsAura.wav");
	RageAscensionSound.setBuffer(RageAscensionSoundBuffer);
	RageAscensionSound.setLoop(false);

	body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
}

void SmashCharacter::ReceiveHeal(int heal) {
	BoulderCreature::ReceiveHeal(heal);
	UpdateHealthBar();
}

void SmashCharacter::UpdateHealthBar() {
	float percent_health = ((float)hit_points / (float)max_hit_points);

	healthBarRect->setSize(sf::Vector2f(max_hit_points * percent_health, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));

	healthBarBackgroundRect->setSize(sf::Vector2f(max_hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);
}

void SmashCharacter::UpdateCharacterExperienceBar() {
	bool leveling_up = characterLevelForDisplay < characterLevel;
	int animated_bar_goal = characterExperienceTowardsNextLevel;
	float bar_x = (characterLevelForDisplay >= 10/* || weaponLevelForDisplay >= 10*/ ? 40.0f : 25.0f);

	if (leveling_up) {
		animated_bar_goal = CharacterExperienceNeededForNextLevel(characterLevelForDisplay);
	}

	if (characterExperienceTowardsNextLevelForAnimatedBar < animated_bar_goal) {
		if (leveling_up) {
			characterExperienceTowardsNextLevelForAnimatedBar += (characterLevel - characterLevelForDisplay) + 1;
		} else {
			characterExperienceTowardsNextLevelForAnimatedBar += 1;
		}
	}
	
	if (leveling_up && characterExperienceTowardsNextLevelForAnimatedBar >= animated_bar_goal) {
		characterLevelForDisplay++;
		characterExperienceTowardsNextLevelForAnimatedBar = 0;
		leveling_up = characterLevelForDisplay < characterLevel;

		if (leveling_up) {
			animated_bar_goal = CharacterExperienceNeededForNextLevel(characterLevelForDisplay);
		}
	}

	if (characterExperienceBarRect == nullptr) {
		characterExperienceBarRect = new sf::RectangleShape(sf::Vector2f((float)animated_bar_goal, 15.0f));
		characterExperienceBarRect->setPosition(bar_x, 40.0f);
		characterExperienceBarRect->setFillColor(sf::Color(255, 153, 153, 255));
	} else {
		characterExperienceBarRect->setSize(sf::Vector2f((float)animated_bar_goal, 15.0f));
		characterExperienceBarRect->setPosition(bar_x, 40.0f);
	}

	if (characterExperienceBarAnimatedRect == nullptr) {
		characterExperienceBarAnimatedRect = new sf::RectangleShape(sf::Vector2f((float)characterExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		characterExperienceBarAnimatedRect->setPosition(bar_x, 40.0f);
		characterExperienceBarAnimatedRect->setFillColor(sf::Color::Red);
	} else {
		characterExperienceBarAnimatedRect->setSize(sf::Vector2f((float)characterExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		characterExperienceBarAnimatedRect->setPosition(bar_x, 40.0f);
	}

	if (characterExperienceBarBackgroundRect == nullptr) {
		characterExperienceBarBackgroundRect = new sf::RectangleShape(sf::Vector2f((float)CharacterExperienceNeededForNextLevel(characterLevelForDisplay), 15.0f));
		characterExperienceBarBackgroundRect->setPosition(bar_x, 40.0f);
		characterExperienceBarBackgroundRect->setFillColor(sf::Color::White);
		characterExperienceBarBackgroundRect->setOutlineThickness(5.0f);
		characterExperienceBarBackgroundRect->setOutlineColor(sf::Color::Black);
	} else {
		characterExperienceBarBackgroundRect->setSize(sf::Vector2f((float)CharacterExperienceNeededForNextLevel(characterLevelForDisplay), 15.0f));
		characterExperienceBarBackgroundRect->setPosition(bar_x, 40.0f);
	}

	if (characterLevelText == nullptr) {
		characterLevelText = new sf::Text(to_string(characterLevelForDisplay), ringbearerFont, 25);
		characterLevelText->setFillColor(sf::Color::White);
		characterLevelText->setOutlineColor(sf::Color::Black);
		characterLevelText->setOutlineThickness(1.5f);
		characterLevelText->setPosition(5.0f, 30.0f);
	} else if (characterLevelText->getString() != to_string(characterLevelForDisplay)) {
		characterLevelText->setString(to_string(characterLevelForDisplay));
	}
}

//void SmashCharacter::UpdateWeaponExperienceBar() {
//	bool leveling_up = weaponLevelForDisplay < weaponLevel;
//	int animated_bar_goal = weaponExperienceTowardsNextLevel;
//	float bar_x = (characterLevelForDisplay >= 10 || weaponLevelForDisplay >= 10 ? 40.0f : 25.0f);
//
//	if (leveling_up) {
//		animated_bar_goal = WeaponExperienceNeededForNextLevel(weaponLevelForDisplay);
//	}
//
//	if (weaponExperienceTowardsNextLevelForAnimatedBar < animated_bar_goal) {
//		if (leveling_up) {
//			weaponExperienceTowardsNextLevelForAnimatedBar += (weaponLevel - weaponLevelForDisplay) + 1;
//		}
//		else {
//			weaponExperienceTowardsNextLevelForAnimatedBar += 1;
//		}
//	}
//
//	if (leveling_up && weaponExperienceTowardsNextLevelForAnimatedBar >= animated_bar_goal) {
//		weaponLevelForDisplay++;
//		weaponExperienceTowardsNextLevelForAnimatedBar = 0;
//		leveling_up = weaponLevelForDisplay < weaponLevel;
//
//		if (leveling_up) {
//			animated_bar_goal = WeaponExperienceNeededForNextLevel(weaponLevelForDisplay);
//		}
//	}
//
//	if (weaponExperienceBarRect == nullptr) {
//		weaponExperienceBarRect = new sf::RectangleShape(sf::Vector2f((float)animated_bar_goal, 15.0f));
//		weaponExperienceBarRect->setPosition(bar_x, 70.0f);
//		weaponExperienceBarRect->setFillColor(sf::Color(153, 153, 255, 255));
//	}
//	else {
//		weaponExperienceBarRect->setSize(sf::Vector2f((float)animated_bar_goal, 15.0f));
//		weaponExperienceBarRect->setPosition(bar_x, 70.0f);
//	}
//
//	if (weaponExperienceBarAnimatedRect == nullptr) {
//		weaponExperienceBarAnimatedRect = new sf::RectangleShape(sf::Vector2f((float)weaponExperienceTowardsNextLevelForAnimatedBar, 15.0f));
//		weaponExperienceBarAnimatedRect->setPosition(bar_x, 70.0f);
//		weaponExperienceBarAnimatedRect->setFillColor(sf::Color::Blue);
//	}
//	else {
//		weaponExperienceBarAnimatedRect->setSize(sf::Vector2f((float)weaponExperienceTowardsNextLevelForAnimatedBar, 15.0f));
//		weaponExperienceBarAnimatedRect->setPosition(bar_x, 70.0f);
//	}
//
//	if (weaponExperienceBarBackgroundRect == nullptr) {
//		weaponExperienceBarBackgroundRect = new sf::RectangleShape(sf::Vector2f((float)WeaponExperienceNeededForNextLevel(weaponLevelForDisplay), 15.0f));
//		weaponExperienceBarBackgroundRect->setPosition(bar_x, 70.0f);
//		weaponExperienceBarBackgroundRect->setFillColor(sf::Color::White);
//		weaponExperienceBarBackgroundRect->setOutlineThickness(5.0f);
//		weaponExperienceBarBackgroundRect->setOutlineColor(sf::Color::Black);
//	}
//	else {
//		weaponExperienceBarBackgroundRect->setSize(sf::Vector2f((float)WeaponExperienceNeededForNextLevel(weaponLevelForDisplay), 15.0f));
//		weaponExperienceBarBackgroundRect->setPosition(bar_x, 70.0f);
//	}
//
//	if (weaponLevelText == nullptr) {
//		weaponLevelText = new sf::Text(to_string(weaponLevelForDisplay), ringbearerFont, 25);
//		weaponLevelText->setFillColor(sf::Color::White);
//		weaponLevelText->setOutlineColor(sf::Color::Black);
//		weaponLevelText->setOutlineThickness(1.5f);
//		weaponLevelText->setPosition(5.0f, 60.0f);
//	} else if (weaponLevelText->getString() != to_string(weaponLevelForDisplay)) {
//		weaponLevelText->setString(to_string(weaponLevelForDisplay));
//	}
//}

void SmashCharacter::UpdateRageLevelBar() {
	if (angerTowardsNextRageLevel > 0) {
		angerTowardsNextRageLevel -= 1;
	} else if (angerTowardsNextRageLevel <= 0 && rageLevel > 0) {
		DecreaseRageLevel();
	} else {
		angerTowardsNextRageLevel = 0;
	}

	rageLevelProgressBarRect->setSize(sf::Vector2f((float)angerTowardsNextRageLevel / (float)angerNeededForNextRageLevel * 200.0f, 15.0f));
}

void SmashCharacter::IncreaseRageLevel() {
	rageLevel++;
	rageLevelProgressBarRect->setFillColor(tierRageColors[rageLevel]);
	rageLevelText->setString(to_string(rageLevel));
	angerTowardsNextRageLevel = (int)(angerNeededForNextRageLevel * 0.25f);

	RageAscensionFadeVolume = RageAscensionStartingVolume;
	RageAscensionSound.play();
	//numberOfRunesYouCanActivate++;
}

void SmashCharacter::DecreaseRageLevel() {
	//int number_of_active_runes = (int)tierActivatedRunes.size();
	//
	//if (number_of_active_runes > 0 || number_of_active_runes > rageLevel) {
	//	tierActivatedRunes[number_of_active_runes - 1]->Equipped = false;
	//	tierActivatedRunes.pop_back();
	//}

	//if (numberOfRunesYouCanActivate > rageLevel) {
	//	numberOfRunesYouCanActivate = rageLevel;
	//}

	rageLevel--;
	rageLevelProgressBarRect->setFillColor(tierRageColors[rageLevel]);
	rageLevelText->setString(to_string(rageLevel));
	angerTowardsNextRageLevel = (int)(angerNeededForNextRageLevel * 0.75f);
}

void SmashCharacter::ResetRuneUiPositions(sf::Vector2f viewport_dimensions) {
	if (DpadLeftRune != nullptr) {
		DpadLeftRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 200.0f, viewport_dimensions.y - 125.0f);
		DpadLeftRune->UiSprite->setPosition(DpadLeftRune->UiPosition);
	}

	if (DpadUpRune != nullptr) {
		DpadUpRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 125.0f, viewport_dimensions.y - 200.0f);
		DpadUpRune->UiSprite->setPosition(DpadUpRune->UiPosition);
	}

	if (DpadRightRune != nullptr) {
		DpadRightRune->UiPosition = sf::Vector2f(viewport_dimensions.x - 50.0f, viewport_dimensions.y - 125.0f);
		DpadRightRune->UiSprite->setPosition(DpadRightRune->UiPosition);
	}
}

void SmashCharacter::UpdateRuneUiItems() {
	readyRuneOffset += (int)((current_frame % 61) - 30) / 120.0f;

	//if (numberOfRunesYouCanActivate > 0) {
	//	if (!DpadLeftRune->Active) {
	//		DpadLeftRune->UiSprite->setPosition(DpadLeftRune->UiPosition.x + readyRuneOffset, DpadLeftRune->UiPosition.y + readyRuneOffset);
	//	}
	//	if (!DpadUpRune->Active) {
	//		DpadUpRune->UiSprite->setPosition(DpadUpRune->UiPosition.x + readyRuneOffset, DpadUpRune->UiPosition.y + readyRuneOffset);
	//	}
	//	if (!DpadRightRune->Active) {
	//		DpadRightRune->UiSprite->setPosition(DpadRightRune->UiPosition.x + readyRuneOffset, DpadRightRune->UiPosition.y + readyRuneOffset);
	//	}
	//}

	if (DpadLeftRune != nullptr) {
		if (DpadLeftRune->Equipped) {
			DpadLeftRune->UiSprite->setScale(0.8f, 0.8f);
		}
		else {
			DpadLeftRune->UiSprite->setScale(0.3f, 0.3f);
		}
	}

	if (DpadUpRune != nullptr) {
		if (DpadUpRune->Equipped) {
			DpadUpRune->UiSprite->setScale(0.8f, 0.8f);
		}
		else {
			DpadUpRune->UiSprite->setScale(0.3f, 0.3f);
		}
	}

	if (DpadRightRune != nullptr) {
		if (DpadRightRune->Equipped) {
			DpadRightRune->UiSprite->setScale(0.8f, 0.8f);
		}
		else {
			DpadRightRune->UiSprite->setScale(0.3f, 0.3f);
		}
	}
}

void SmashCharacter::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies) {
	bool was_alive = hit_points > 0;

	if (DefenseRune->Equipped) {
		damage = (int)(damage * 0.75f);
	}

	BoulderCreature::TakeDamage(damage, knock_back, hit_stun_frames, pop_up_grounded_enemies);
	UpdateHealthBar();

	if (was_alive && hit_points <= 0) {
		Singleton<SmashWorld>::Get()->PlayerDied();
	}
}

void SmashCharacter::ApplyObjectDataToSaveData(Json::Value& save_data) {
	BoulderCreature::ApplyObjectDataToSaveData(save_data["Player"]);

	save_data["Player"]["CharacterLevel"] = characterLevel;
	save_data["Player"]["MaximumHitPoints"] = max_hit_points;
	save_data["Player"]["CharacterExperienceTowardsNextLevel"] = characterExperienceTowardsNextLevel;
	save_data["Player"]["RageLevel"] = rageLevel;
	save_data["Player"]["AngerTowardsNextRageLevel"] = angerTowardsNextRageLevel;
	save_data["Player"]["DpadLeftRuneName"] = DpadLeftRune != nullptr ? DpadLeftRune->Name : "";
	save_data["Player"]["DpadUpRuneName"] = DpadUpRune != nullptr ? DpadUpRune->Name : "";
	save_data["Player"]["DpadRightRuneName"] = DpadRightRune != nullptr ? DpadRightRune->Name : "";
	//save_data["Player"]["WeaponLevel"] = weaponLevel;
	//save_data["Player"]["WeaponExperienceTowardsNextLevel"] = weaponExperienceTowardsNextLevel;
}

void SmashCharacter::ApplySaveDataToObjectData(Json::Value& save_data) {
	BoulderCreature::ApplySaveDataToObjectData(save_data["Player"]);

	characterLevel = characterLevelForDisplay = save_data["Player"]["CharacterLevel"].asInt();
	max_hit_points = GetMaximumHitPointsFromLevel(characterLevel);
	characterExperienceTowardsNextLevel = save_data["Player"]["CharacterExperienceTowardsNextLevel"].asInt();
	rageLevel = save_data["Player"]["RageLevel"].asInt();
	angerTowardsNextRageLevel = save_data["Player"]["AngerTowardsNextRageLevel"].asInt();
	rageLevelText->setString(to_string(rageLevel));
	rageLevelProgressBarRect->setFillColor(tierRageColors[rageLevel]);

	string dpad_left_rune_name = save_data["Player"]["DpadLeftRuneName"].asString();
	string dpad_up_rune_name = save_data["Player"]["DpadUpRuneName"].asString();
	string dpad_right_rune_name = save_data["Player"]["DpadRightRuneName"].asString();

	int runes_list_size = (int)OwnedRunesList.size();
	for (int i = 0; i < runes_list_size; i++) {
		if (OwnedRunesList[i]->Name == dpad_left_rune_name) {
			DpadLeftRune = OwnedRunesList[i];
		} else if (OwnedRunesList[i]->Name == dpad_up_rune_name) {
			DpadUpRune = OwnedRunesList[i];
		} else if (OwnedRunesList[i]->Name == dpad_right_rune_name) {
			DpadRightRune = OwnedRunesList[i];
		}
	}
	//weaponExperienceTowardsNextLevel = save_data["Player"]["WeaponExperienceTowardsNextLevel"].asInt();
	//weaponLevelForDisplay = weaponLevel = save_data["Player"]["WeaponLevel"].asInt();

	UpdateHealthBar();
	UpdateCharacterExperienceBar();

	if (DpadLeftRune != nullptr) {
		DpadLeftRune->Equipped = true;
	}
	if (DpadUpRune != nullptr) {
		DpadUpRune->Equipped = false;
	}
	if (DpadRightRune != nullptr) {
		DpadRightRune->Equipped = false;
	}

	if (GetNumberOfRuneSlotsFromLevel(characterLevel) >= 3) {
		if (DpadUpRune != nullptr) {
			DpadUpRune->Equipped = true;
		}
		if (DpadRightRune != nullptr) {
			DpadRightRune->Equipped = true;
		}
	} else if (GetNumberOfRuneSlotsFromLevel(characterLevel) >= 2) {
		if (DpadUpRune != nullptr) {
			DpadUpRune->Equipped = true;
		}
	}

	ResetRuneUiPositions(Singleton<SmashWorld>().Get()->GetCamera()->viewport_dimensions);
	//UpdateWeaponExperienceBar();
}

void SmashCharacter::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	b2Vec2 body_lin_vel = body->GetLinearVelocity();

	if ((int)platformContacts.size() > 0) {
		b2Vec2 platform_lin_vel = platformContacts[0]->GetBody()->GetLinearVelocity();

		body->SetLinearVelocity(b2Vec2(body_lin_vel.x + platform_lin_vel.x, body_lin_vel.y));
	}

	UpdateCharacterExperienceBar();
	//UpdateWeaponExperienceBar();
	UpdateRageLevelBar();
	UpdateRuneUiItems();

	if (RageAscensionSound.getStatus() == sf::Sound::Playing) {
		RageAscensionFadeVolume -= 0.01f * RageAscensionStartingVolume;

		if (RageAscensionFadeVolume < 0.0f) {
			RageAscensionSound.stop();
		} else {
			RageAscensionSound.setVolume(RageAscensionFadeVolume);
		}
	}

	if (IsInTheAir()) {
		b2Vec2 vel = body->GetLinearVelocity();

		if (leftStickInputs.y > 90.0f && vel.y > 0) {
			fastFalling = true;
		}

		if (fastFalling) {
			body->SetLinearVelocity(b2Vec2(vel.x, fastFallingVelocity));
			maxVerticalVelocityReached = fastFallingVelocity;
		} else if (vel.y > normalTerminalVelocity) {
			body->SetLinearVelocity(b2Vec2(vel.x, normalTerminalVelocity));
		}
	}
	
	if (jumpStartUpTimerWasActiveLastFrame && !jumpStartUpTimer->IsActive()) {
		ActuallyJump(releasedJumpButton);
	} else if (anAttackWasActiveLastFrame && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		if (jumpInputBufferWasActiveLastFrame && !jumpInputBuffer->IsActive()) {
			StartJump();
		} else if (attack_input_buffer->IsActive()) {
			UseAttack(attack_buffer_attack_index, false);
		}
	}

	jumpInputBufferWasActiveLastFrame = jumpInputBuffer->IsActive();
	jumpStartUpTimerWasActiveLastFrame = jumpStartUpTimer->IsActive();
	anAttackWasActiveLastFrame = IsAnAttackActive();


	can_take_input = !hit_stun_timer->IsActive();
	//if (can_take_input) {
	//	can_take_input = !IsAnAttackActive();
	//}

	hurt_box->Update(IsFacingRight());

	//if (body->GetLinearVelocity().x > 0.1f && !IsFacingRight()) {
	//	SetFacingRight(true);
	//} else if (body->GetLinearVelocity().x < -0.1f && IsFacingRight()) {
	//	SetFacingRight(false);
	//}

	for (int i = 0; i < numberOfAttacks; i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	weapon->Update(curr_frame, delta_time);

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	} else {
		topCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	}

	if (hit_points <= 0) {
		if (dying_animation_timer != nullptr && dying_animation_timer->IsActive()) {
			State = STATE_DYING;
		} else {
			State = STATE_DEAD;
		}
	} else if (hit_stun_timer->IsActive()) {
		State = STATE_HIT_STUN;
	} else if (IsAnAttackActive()) {
		State = STATE_ATTACKING;
	} else if (IsInTheAir()) {
		if (body->GetLinearVelocity().y < -1.0f) {
			State = STATE_JUMPING;
		} else if (body->GetLinearVelocity().y > 1.0f) {
			State = STATE_FALLING;
		} else {
			State = STATE_JUMP_APEX;
		}
	} else if (landing_animation_timer->IsActive()) {
		State = STATE_LANDING;
	} else if (leftStickInputs.x != 0) {
		if (running) {
			State = STATE_RUNNING;
		} else {
			State = STATE_WALKING;
		}
	} else {
		State = STATE_IDLE;
	}

	if (State == STATE_WALKING) {
		if (walking_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameWalk) {
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameWalk) {
			LeftFootStepSound.play();
		}
	} else if (State == STATE_RUNNING) {
		if (running_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameRun) {
			RightFootStepSound.play();
		} else if (running_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameRun) {
			LeftFootStepSound.play();
		}
	} else if (State == STATE_ATTACKING) {
		int currentAttackIndex = GetActiveAttackIndex();
		if (currentAttackIndex < (int)AttackAnimationSounds.size()) {
			if (attacking_animations[currentAttackIndex]->GetCurrentFrame() == AttackAnimationSoundFrames[currentAttackIndex]) {
				AttackAnimationSounds[currentAttackIndex]->play();
			}
		}
	}
}

void SmashCharacter::DetermineWhichAttackToUseAndActivateIt(float x_input, float y_input) {
	if (IsInTheAir()) {
		if (y_input < -90.0f) {
			UseAttack(Attack::UP_AIR);
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_AIR);
		} else if (x_input < -90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::BACK_AIR);
			} else {
				UseAttack(Attack::FORWARD_AIR);
			}
		} else if (x_input > 90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::FORWARD_AIR);
			} else {
				UseAttack(Attack::BACK_AIR);
			}
		} else {
			UseAttack(Attack::NEUTRAL_AIR);
		}
	} else {
		if (y_input < -90.0f) {
			UseAttack(Attack::UP_SMASH);
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_SMASH);
		} else if (x_input < -90.0f) {
			if (IsFacingRight() && !IsAnAttackActive()) {
				SetFacingRight(false);
			}
			attacks[Attack::FORWARD_SMASH]->SetFacingRight(IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
		} else if (x_input > 90.0f) {
			if (!IsFacingRight() && !IsAnAttackActive()) {
				SetFacingRight(true);
			}
			attacks[Attack::FORWARD_SMASH]->SetFacingRight(IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
		} else {
			UseAttack(Attack::JAB);
		}
	}
}

void SmashCharacter::Draw(sf::Vector2f camera_position) {
	if (rageLevel > 0) {
		rageTierAuraAnimations[rageLevel - 1]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), halfBodyHeight);
	}

	render_window->draw(*healthBarBackgroundRect);
	render_window->draw(*characterExperienceBarBackgroundRect);
	render_window->draw(*characterExperienceBarRect); 
	render_window->draw(*characterExperienceBarAnimatedRect);
	//render_window->draw(*weaponExperienceBarBackgroundRect);
	//render_window->draw(*weaponExperienceBarRect);
	//render_window->draw(*weaponExperienceBarAnimatedRect);
	render_window->draw(*rageLevelBarBackgroundRect);
	render_window->draw(*rageLevelProgressBarRect);

	render_window->draw(*characterLevelText);
	//render_window->draw(*weaponLevelText);
	render_window->draw(*rageLevelText);

	render_window->draw(*DpadSprite);

	if (DpadLeftRune != nullptr) {
		render_window->draw(*DpadLeftRune->UiSprite);
	}

	if (GetNumberOfRuneSlotsFromLevel(characterLevel) >= 3) {
		if (DpadUpRune != nullptr) {
			render_window->draw(*DpadUpRune->UiSprite);
		}
		if (DpadRightRune != nullptr) {
			render_window->draw(*DpadRightRune->UiSprite);
		}
	}
	else if (GetNumberOfRuneSlotsFromLevel(characterLevel) >= 2) {
		render_window->draw(*DpadUpRune->UiSprite);
	}

	BoulderCreature::Draw(camera_position);

	//Set position to Center.X + Radius * cos(Period), Center.Y + Radius * sin(Period)
	//Add 1 to Period
	//If Period = 360, set it to 0
	//Rinse, repeat

	//b2Vec2 pos = body->GetPosition();

	//if (DpadLeftRune->Equipped) {
	//	orbitPeriod1 = orbitPeriod1 + 0.02f;
	//	if (orbitPeriod1 >= 360.0f) { orbitPeriod1 = 0.0f; }
	//	DpadLeftRune->InGameSprite->setPosition(sf::Vector2f(((pos.x - camera_position.x) * 40.0f - 7.5f) + 25.0f * cos(orbitPeriod1), ((pos.y - camera_position.y) * 40.0f - 7.5f) + 25.0f * sin(orbitPeriod1)));
	//	render_window->draw(*DpadLeftRune->InGameSprite);
	//}
	//if (DpadUpRune->Equipped) {
	//	orbitPeriod2 = orbitPeriod2 + 0.04f;
	//	if (orbitPeriod2 >= 360.0f) { orbitPeriod2 = 0.0f; }
	//	DpadUpRune->InGameSprite->setPosition(sf::Vector2f(((pos.x - camera_position.x) * 40.0f - 7.5f) + 22.0f * cos(orbitPeriod2), ((pos.y - camera_position.y) * 40.0f - 7.5f) + 22.0f * sin(orbitPeriod2)));
	//	render_window->draw(*DpadUpRune->InGameSprite);
	//}
	//if (DpadRightRune->Equipped) {
	//	orbitPeriod3 = orbitPeriod3 + 0.06f;
	//	if (orbitPeriod3 >= 360.0f) { orbitPeriod3 = 0.0f; }
	//	DpadRightRune->InGameSprite->setPosition(sf::Vector2f(((pos.x - camera_position.x) * 40.0f - 7.5f) + 19.0f * cos(orbitPeriod3), ((pos.y - camera_position.y) * 40.0f - 7.5f) + 19.0f * sin(orbitPeriod3)));
	//	render_window->draw(*DpadRightRune->InGameSprite);
	//}
}

void SmashCharacter::HandleButtonXPress() {
	if (interactable != nullptr) {
		interactable->StartTalking();
		Singleton<SmashWorld>::Get()->StartDialogue(interactable->GetType());
	} else {
		DetermineWhichAttackToUseAndActivateIt(leftStickInputs.x, leftStickInputs.y);
	}
}

void SmashCharacter::DashPunch() {
}

void SmashCharacter::ThrowWeapon() {
	if (IsFacingRight()) {
		weapon->Throw(b2Vec2(5.0f + body->GetLinearVelocity().x, -5.0f + body->GetLinearVelocity().y), body->GetPosition());
	} else {
		weapon->Throw(b2Vec2(-5.0f + body->GetLinearVelocity().x, -5.0f + body->GetLinearVelocity().y), body->GetPosition());
	}
}

void SmashCharacter::TeleportToWeapon() {
	if (weapon->CanTeleportToWeapon() && !teleportedSinceLastLanding) {
		if (IsInTheAir()) {
			teleportedSinceLastLanding = true;
		}

		b2Vec2 original_position = body->GetPosition();
		b2Vec2 new_position = weapon->GetBody()->GetPosition();

		b2Vec2 travel_vector = b2Vec2(new_position.x - original_position.x, new_position.y - original_position.y);
		travel_vector.Normalize();
		float32 speed = 10.0f;

		body->SetTransform(weapon->GetBody()->GetPosition(), body->GetAngle());
		body->SetLinearVelocity(b2Vec2(travel_vector.x * speed, travel_vector.y * speed));

		weapon->TeleportedToWeapon();
	}
}

void SmashCharacter::HandleButtonSelectPress() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectPress();
}

void SmashCharacter::HandleButtonSelectRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectRelease();
}

void SmashCharacter::HandleButtonStartPress() {
	Singleton<SmashWorld>::Get()->HandleButtonStartPress();
}

void SmashCharacter::HandleButtonStartRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonStartRelease();
}

void SmashCharacter::ForcedRecall() {
	weapon->ForcedRecall();
}

void SmashCharacter::Land() {
	teleportedSinceLastLanding = false;

	BoulderCreature::Land();
}

int SmashCharacter::CharacterExperienceNeededForNextLevel(int level) {
	return 200 + (40 * level);
}

//int SmashCharacter::WeaponExperienceNeededForNextLevel(int level) {
//	return 100 + (10 * level);
//}

void SmashCharacter::ReceiveExperience(int experience_points) {
	characterExperienceTowardsNextLevel += experience_points;
	//weaponExperienceTowardsNextLevel += experience_points;

	while (characterExperienceTowardsNextLevel >= CharacterExperienceNeededForNextLevel(characterLevel)) {
		LevelUpCharacter();
	}

	//while (weaponExperienceTowardsNextLevel >= WeaponExperienceNeededForNextLevel(weaponLevel)) {
	//	LevelUpWeapon();
	//}

	UpdateCharacterExperienceBar();
	//UpdateWeaponExperienceBar();
}

void SmashCharacter::LevelUpCharacter() {
	characterExperienceTowardsNextLevel -= CharacterExperienceNeededForNextLevel(characterLevel);
	characterLevel++;

	max_hit_points = GetMaximumHitPointsFromLevel(characterLevel);
	hit_points = max_hit_points;

	healthBarRect = new sf::RectangleShape(sf::Vector2f(hit_points, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);
}

//void SmashCharacter::LevelUpWeapon() {
//	weaponExperienceTowardsNextLevel -= WeaponExperienceNeededForNextLevel(weaponLevel);
//	weaponLevel++;
//}


int SmashCharacter::GetMaximumHitPointsFromLevel(int char_level) {
	int extra_health = 0;

	if (char_level >= 8) {
		extra_health = 400;
	} else if (char_level >= 6) {
		extra_health = 300;
	} else if (char_level >= 3) {
		extra_health = 200;
	} else if (char_level >= 1) {
		extra_health = 100;
	}

	return baseMaxHitPoints + extra_health;
}

int SmashCharacter::GetNumberOfRuneSlotsFromLevel(int char_level) {
	if (char_level >= 10) {
		return 3;
	} else if (char_level >= 5) {
		return 2;
	}

	return 1;
}

float SmashCharacter::GetDamageMultiplierFromLevel(int char_level) {
	if (char_level >= 9) {
		return 5.0f;
	} else if (char_level >= 7) {
		return 4.0f;
	} else if (char_level >= 4) {
		return 3.0f;
	} else if (char_level >= 2) {
		return 2.0f;
	}

	return 1.0f;
}

int SmashCharacter::GetDamageOfCurrentAttack() {
	Attack* active_attack = GetActiveAttack();

	if (active_attack != nullptr) {
		int damage = active_attack->GetDamage();

		if (BerserkerRune->Equipped) {
			damage = (int)(2.0f - (1.5f * (hit_points / max_hit_points)));
		}

		if (DamageRune->Equipped) {
			damage = (int)(damage * 1.5f);
		}

		return (int)((float)damage * ((float)(rageLevel + 1) / 2.0f) * GetDamageMultiplierFromLevel(characterLevel));
	}

	return 0;
}

void SmashCharacter::UpdateEffectsVolumes(float new_effects_volume) {
	BoulderCreature::UpdateEffectsVolumes(new_effects_volume);

	for (int i = 0; i < (int)AttackAnimationSounds.size(); i++) {
		AttackAnimationSounds[i]->setVolume(new_effects_volume);
	}

	RageAscensionStartingVolume = new_effects_volume;
}

void SmashCharacter::AddAnger(int anger_amount) {
	angerTowardsNextRageLevel += anger_amount;

	if (angerTowardsNextRageLevel >= angerNeededForNextRageLevel) {
		if (rageLevel >= 3) {
			angerTowardsNextRageLevel = angerNeededForNextRageLevel;
		} else {
			IncreaseRageLevel();
		}
	}

	if (LifestealRune->Equipped) {
		ReceiveHeal(25);
	}
}

void SmashCharacter::HandleDpadRightPress() {
	//if (numberOfRunesYouCanActivate > 0) {
	//	DpadRightRune->Active = true;
	//	numberOfRunesYouCanActivate--;
	//	tierActivatedRunes.push_back(DpadRightRune);
	//}
}

void SmashCharacter::HandleDpadRightRelease() {
}

void SmashCharacter::HandleDpadLeftPress() {
	//if (numberOfRunesYouCanActivate > 0) {
	//	DpadLeftRune->Active = true;
	//	numberOfRunesYouCanActivate--;
	//	tierActivatedRunes.push_back(DpadLeftRune);
	//}
}

void SmashCharacter::HandleDpadLeftRelease() {
}

void SmashCharacter::HandleDpadUpPress() {
	//if (numberOfRunesYouCanActivate > 0) {
	//	DpadUpRune->Active = true;
	//	numberOfRunesYouCanActivate--;
	//	tierActivatedRunes.push_back(DpadUpRune);
	//}
}

void SmashCharacter::HandleDpadUpRelease() {
}

void SmashCharacter::HandleDpadDownPress() {
}

void SmashCharacter::HandleDpadDownRelease() {
}

void SmashCharacter::ActuallyJump(bool short_hop) {
	if (can_take_input && hit_points > 0 && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		bool jumping = false;

		if (!IsInTheAir() || jumpAfterWalkingOffLedgeBuffer->IsActive()) {
			jumping = true;
			jumpAfterWalkingOffLedgeBuffer->Stop();
		} else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			maxAirSpeed = body->GetLinearVelocity().x;
			float jump_power_accounting_for_rune = (SuperJumpRune->Equipped ? jump_power * 1.5f : jump_power);

			body->SetLinearVelocity(b2Vec2(maxAirSpeed, short_hop ? -jump_power_accounting_for_rune * 0.8f : -jump_power_accounting_for_rune));
			SetInTheAir(true);
		}
	}
}

void SmashCharacter::PickUpRune(string rune_name) {
	int owned_runes_list_size = (int)OwnedRunesList.size();
	for (int i = 0; i < owned_runes_list_size; i++) {
		if (OwnedRunesList[i]->Name == rune_name) {
			return;
		}
	}

	int master_rune_list_size = (int)MasterRunesList.size();
	for (int i = 0; i < master_rune_list_size; i++) {
		if (MasterRunesList[i]->Name == rune_name) {
			OwnedRunesList.push_back(MasterRunesList[i]);

			int number_of_rune_slots_by_level = GetNumberOfRuneSlotsFromLevel(characterLevel);

			if (number_of_rune_slots_by_level == 1) {
				if (DpadLeftRune == nullptr) {
					DpadLeftRune = MasterRunesList[i];
					DpadLeftRune->Equipped = true;
				}
			} else if (number_of_rune_slots_by_level == 2) {
				if (DpadLeftRune == nullptr) {
					DpadLeftRune = MasterRunesList[i];
					DpadLeftRune->Equipped = true;
				} else if(DpadUpRune == nullptr) {
					DpadUpRune = MasterRunesList[i];
					DpadUpRune->Equipped = true;
				}
			} else if (number_of_rune_slots_by_level >= 3) {
				if (DpadLeftRune == nullptr) {
					DpadLeftRune = MasterRunesList[i];
					DpadLeftRune->Equipped = true;
				} else if (DpadUpRune == nullptr) {
					DpadUpRune = MasterRunesList[i];
					DpadUpRune->Equipped = true;
				} else if (DpadRightRune == nullptr) {
					DpadRightRune = MasterRunesList[i];
					DpadRightRune->Equipped = true;
				}
			}

			break;
		}
	}

	ResetRuneUiPositions(Singleton<SmashWorld>::Get()->GetCamera()->viewport_dimensions);
}