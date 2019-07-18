#include "pch.h"
#include "StringHelpers.h"
#include "Game.h"
#include "EntityManager.h"

const float Game::PlayerSpeed = 200.f;
const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game()
	: mWindow(sf::VideoMode(1240, 600), "Zaxxon", sf::Style::Close)
	, mTexture()
	, mPlayer()
	, mFont()
	, mStatisticsText()
	, mStatisticsUpdateTime()
	, mStatisticsNumFrames(0)
	, mIsMovingUp(false)
	, mIsMovingDown(false)
	, mIsMovingRight(false)
	, mIsMovingLeft(false)
{
	mWindow.setFramerateLimit(160);

	_Map.loadFromFile("Media/Textures/bg.jpg");
	_TextureWeapon.loadFromFile("Media/Textures/missile.png");
	_TextureWeaponEnemy.loadFromFile("Media/Textures/rocket.png");
	_TextureWeaponEnemyMaster.loadFromFile("Media/Textures/badshot.png");
	mTexture.loadFromFile("Media/Textures/fly.png");
	_TextureEnemyMaster.loadFromFile("Media/Textures/boss.png");
	_TextureEnemy.loadFromFile("Media/Textures/tank.png");
	_TextureBlock.loadFromFile("Media/Textures/bomker2.png");
	_TextureBoom.loadFromFile("Media/Textures/boom.gif");
	mFont.loadFromFile("Media/Sansation.ttf");

	InitSprites();
}

void Game::ResetSprites()
{
	_IsGameOver = false;
	_IsEnemyWeaponFired = false;
	_IsPlayerWeaponFired = false;
	_IsEnemyMasterWeaponFired = false;

	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		entity->m_enabled = true;
	}
}

void Game::InitSprites()
{
	_lives = 3;
	_score = 0;
	_bosslife = 300;
	_IsGameOver = false;
	_IsEnemyWeaponFired = false;
	_IsPlayerWeaponFired = false;
	_IsEnemyMasterWeaponFired = false;

	//
	// map
	//
	std::shared_ptr<Entity> sw = std::make_shared<Entity>();
	sw->m_sprite.setTexture(_Map);
	sw->m_sprite.setPosition(0.f, 0.f);
	sw->m_type = EntityType::map;
	sw->m_size = _Map.getSize();
	EntityManager::m_Entities.push_back(sw);


	for (int i = 0; i < SPRITE_COUNT_X; i++)
	{
		for (int j = 0; j < SPRITE_COUNT_Y; j++)
		{
			int r = ((double)rand() / (RAND_MAX)) + 1;
			if (r == 1) {
				_Enemy[i][j].setTexture(_TextureEnemy);
				_Enemy[i][j].setPosition(900.f + 50.f * (j + 1), 50.f * (i + 1));

				std::shared_ptr<Entity> se = std::make_shared<Entity>();
				se->m_sprite = _Enemy[i][j];
				se->m_type = EntityType::enemy;
				se->m_size = _TextureEnemy.getSize();
				se->m_position = _Enemy[i][j].getPosition();
				EntityManager::m_Entities.push_back(se);
			}

		}
	}

	//
	// Blocks
	//

	for (int i = 0; i < BLOCK_COUNT_X; i++)
	{
		for (int j = 0; j < BLOCK_COUNT_Y; j++)
		{
			_Block[i][j].setTexture(_TextureBlock);
			_Block[i][j].setPosition(400.f + 200.f * (j + 1), 150.f * (i + 1));

			std::shared_ptr<Entity> sb = std::make_shared<Entity>();
			sb->m_sprite = _Block[i][j];
			sb->m_type = EntityType::block;
			sb->m_size = _TextureBlock.getSize();
			sb->m_position = _Block[i][j].getPosition();
			EntityManager::m_Entities.push_back(sb);

		}
	}
	//
	// Enemy Master
	//

	_EnemyMaster.setTexture(_TextureEnemyMaster);
	_EnemyMaster.setPosition(1000.f + 100.f, 100.f);
	std::shared_ptr<Entity> sem = std::make_shared<Entity>();
	sem->m_sprite = _EnemyMaster;
	sem->m_type = EntityType::enemyMaster;
	sem->m_size = _TextureEnemyMaster.getSize();
	sem->m_position = _EnemyMaster.getPosition();
	EntityManager::m_Entities.push_back(sem);


	//
	// Player
	//

	mPlayer.setTexture(mTexture);
	mPlayer.setPosition(10.f, 250.f);
	std::shared_ptr<Entity> player = std::make_shared<Entity>();
	player->m_sprite = mPlayer;
	player->m_type = EntityType::player;
	player->m_size = mTexture.getSize();
	player->m_position = mPlayer.getPosition();
	EntityManager::m_Entities.push_back(player);


	mStatisticsText.setFont(mFont);
	mStatisticsText.setPosition(5.f, 5.f);
	mStatisticsText.setCharacterSize(10);

	//
	// Lives
	//

	_LivesText.setFillColor(sf::Color::Green);
	_LivesText.setFont(mFont);
	_LivesText.setPosition(10.f, 30.f);
	_LivesText.setCharacterSize(20);
	_LivesText.setString(std::to_string(_lives));

	//
	// Text
	//

	_ScoreText.setFillColor(sf::Color::Green);
	_ScoreText.setFont(mFont);
	_ScoreText.setPosition(10.f, 50.f);
	_ScoreText.setCharacterSize(20);
	_ScoreText.setString(std::to_string(_score));

	//
	// bossLife
	//
	_BossLifeText.setFillColor(sf::Color::Red);
	_BossLifeText.setFont(mFont);
	_BossLifeText.setPosition(1000.f + 100.f, 10.f);
	_BossLifeText.setCharacterSize(20);
	_BossLifeText.setString(std::to_string(_bosslife));

}

void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	while (mWindow.isOpen())
	{
		sf::Time elapsedTime = clock.restart();
		timeSinceLastUpdate += elapsedTime;
		while (timeSinceLastUpdate > TimePerFrame)
		{
			timeSinceLastUpdate -= TimePerFrame;

			processEvents();
			update(TimePerFrame);
		}

		updateStatistics(elapsedTime);
		render();
	}
}

void Game::processEvents()
{
	sf::Event event;
	while (mWindow.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			handlePlayerInput(event.key.code, true);
			break;

		case sf::Event::KeyReleased:
			handlePlayerInput(event.key.code, false);
			break;

		case sf::Event::Closed:
			mWindow.close();
			break;
		}
	}
}

void Game::update(sf::Time elapsedTime)
{
	sf::Vector2f movement(0.f, 0.f);
	if (mIsMovingUp) {
		movement.y -= PlayerSpeed;
	}
	if (mIsMovingDown)
		movement.y += PlayerSpeed;
	if (mIsMovingLeft)
		movement.x -= PlayerSpeed;
	if (mIsMovingRight)
		movement.x += PlayerSpeed;

	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::player)
		{
			continue;
		}

		entity->m_sprite.move(movement * elapsedTime.asSeconds());
	}
}

void Game::render()
{
	mWindow.clear();

	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		mWindow.draw(entity->m_sprite);
	}

	mWindow.draw(mStatisticsText);
	mWindow.draw(mText);
	mWindow.draw(_LivesText);
	mWindow.draw(_ScoreText);
	mWindow.draw(_BossLifeText);
	mWindow.display();
}

void Game::updateStatistics(sf::Time elapsedTime)
{
	mStatisticsUpdateTime += elapsedTime;
	mStatisticsNumFrames += 1;

	if (mStatisticsUpdateTime >= sf::seconds(1.0f))
	{
		mStatisticsText.setString(
			"Frames / Second = " + toString(mStatisticsNumFrames) + "\n" +
			"Time / Update = " + toString(mStatisticsUpdateTime.asMicroseconds() / mStatisticsNumFrames) + "us");

		mStatisticsUpdateTime -= sf::seconds(1.0f);
		mStatisticsNumFrames = 0;
	}

	//
	// Handle collisions
	//

	if (mStatisticsUpdateTime >= sf::seconds(0.050f))
	{
		if (_IsGameOver == true)
			return;

		HandleTexts();
		HandleGameOver();
		HandleCollisionWeaponEnemy();
		HandleCollisionWeaponPlayer();
		HandleCollisionWeaponBlock();
		HandleCollisionEnemyWeaponBlock();
		HandleCollisionEnemyMasterWeaponBlock();
		HandleCollisionEnemyMasterWeaponPlayer();
		HandleCollisionBlockEnemy();
		HandleCollisionPlayerBlocks();
		HandleCollisionWeaponEnemyMaster();
		HanldeWeaponMoves();
		HanldeEnemyWeaponMoves();
		HanldeEnemyMasterWeaponMoves();
		HandleEnemyMoves();
		HandleEnemyMasterMove();
		HandleEnemyWeaponFiring();
		HandleEnemyMasterWeaponFiring();
		HandleBox();
	}
}

void Game::HandleTexts()
{
	std::string lives = "Lives: " + std::to_string(_lives);
	_LivesText.setString(lives);
	std::string score = "Score: " + std::to_string(_score);
	_ScoreText.setString(score);
	std::string bosslife = "Boss life: " + std::to_string(_bosslife);
	_BossLifeText.setString(bosslife);
	return;
}

void Game::HandleCollisionEnemyMasterWeaponPlayer()
{
	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::enemyMasterWeapon)
		{
			continue;
		}

		sf::FloatRect boundWeapon;
		boundWeapon = weapon->m_sprite.getGlobalBounds();

		sf::FloatRect boundPlayer;
		boundPlayer = EntityManager::GetPlayer()->m_sprite.getGlobalBounds();

		if (boundWeapon.intersects(boundPlayer) == true)
		{
			weapon->m_enabled = false;
			_IsEnemyMasterWeaponFired = false;
			_lives--;
			//break;
			goto end;
		}
	}

end:
	//nop
	return;
}

void Game::HanldeEnemyMasterWeaponMoves()
{
	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::enemyMasterWeapon)
		{
			continue;
		}

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;
		x--;

		if (x <= 0)
		{
			entity->m_enabled = false;
			_IsEnemyMasterWeaponFired = false;
		}

		entity->m_sprite.setPosition(x, y);
	}
}

void Game::HandleEnemyMasterWeaponFiring()
{
	if (_IsEnemyMasterWeaponFired == true)
		return;

	if (EntityManager::GetEnemyMaster()->m_enabled == false)
		return;

	// a little random...
	int r = rand() % 50;
	if (r != 10)
		return;

	float x, y;
	x = EntityManager::GetEnemyMaster()->m_sprite.getPosition().x;
	y = EntityManager::GetEnemyMaster()->m_sprite.getPosition().y;
	y--;

	std::shared_ptr<Entity> sw = std::make_shared<Entity>();
	sw->m_sprite.setTexture(_TextureWeaponEnemyMaster);

	sw->m_sprite.setPosition(
		x + _TextureEnemyMaster.getSize().x / 2,
		y + _TextureEnemyMaster.getSize().y);
	sw->m_type = EntityType::enemyMasterWeapon;
	sw->m_size = _TextureWeaponEnemyMaster.getSize();
	EntityManager::m_Entities.push_back(sw);

	_IsEnemyMasterWeaponFired = true;
}

void Game::HandleCollisionEnemyMasterWeaponBlock()
{
	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::enemyMasterWeapon)
		{
			continue;
		}

		for (std::shared_ptr<Entity> block : EntityManager::m_Entities)
		{
			if (block->m_type != EntityType::block)
			{
				continue;
			}

			if (block->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundWeapon;
			boundWeapon = weapon->m_sprite.getGlobalBounds();

			sf::FloatRect boundBlock;
			boundBlock = block->m_sprite.getGlobalBounds();
			/*
			if (boundWeapon.intersects(boundBlock) == true)
			{
				weapon->m_enabled = false;
				_IsEnemyMasterWeaponFired = false;
				//break;
				goto end2;
			}
			*/
		}
	}

end2:
	//nop
	return;
}


void Game::HandleEnemyMasterMove()
{
	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::enemyMaster)
		{
			continue;
		}

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;

		if (entity->m_bLeftToRight == true)
			y = y + 0.5f;
		else
			y = y - 0.5f;

		entity->m_times++;

		if (y >= ((BLOCK_COUNT_X) * 80) || y <= 100)
		{
			if (entity->m_bLeftToRight == true)
			{
				entity->m_bLeftToRight = false;
				entity->m_times = 0;
			}
			else
			{
				entity->m_bLeftToRight = true;
				entity->m_times = 0;
			}
		}

		entity->m_sprite.setPosition(x, y);
	}
}

void Game::HandleCollisionEnemyWeaponBlock()
{
	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::enemyWeapon)
		{
			continue;
		}

		for (std::shared_ptr<Entity> block : EntityManager::m_Entities)
		{
			if (block->m_type != EntityType::block)
			{
				continue;
			}

			if (block->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundWeapon;
			boundWeapon = weapon->m_sprite.getGlobalBounds();

			sf::FloatRect boundBlock;
			boundBlock = block->m_sprite.getGlobalBounds();

			if (boundWeapon.intersects(boundBlock) == true)
			{
				weapon->m_enabled = false;
				_IsEnemyWeaponFired = false;
				//break;
				goto end2;
			}
		}
	}

end2:
	//nop
	return;
}

void Game::HandleCollisionWeaponPlayer()
{
	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::enemyWeapon)
		{
			continue;
		}

		sf::FloatRect boundWeapon;
		boundWeapon = weapon->m_sprite.getGlobalBounds();

		sf::FloatRect boundPlayer;
		boundPlayer = EntityManager::GetPlayer()->m_sprite.getGlobalBounds();

		if (boundWeapon.intersects(boundPlayer) == true)
		{
			weapon->m_enabled = false;
			_IsEnemyWeaponFired = false;
			_lives--;
			goto end;
		}
	}

end:
	//nop
	return;
}

void Game::HandleBox()
{

	sf::FloatRect boundBox;
	boundBox = sf::FloatRect(0, 0, 1240, 600);

	sf::FloatRect boundPlayer;
	float x = EntityManager::GetPlayer()->m_sprite.getPosition().x;
	float y = EntityManager::GetPlayer()->m_sprite.getPosition().y;

	if (y < boundBox.top)
	{
		EntityManager::GetPlayer()->m_sprite.setPosition(x, 600);
		goto end;
	}
	if (x > boundBox.width)
	{
		EntityManager::GetPlayer()->m_sprite.setPosition(0, y);
		goto end;
	}
	/*if (!boundBox.contains(x,y) == true)
	{
		EntityManager::GetPlayer()->m_sprite.setPosition(x,600);
		goto end;
	}*/


end:
	//nop
	return;

}

void Game::HanldeEnemyWeaponMoves()
{
	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::enemyWeapon)
		{
			continue;
		}

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;
		x -= 1.0f;

		if (x <= 0)
		{
			entity->m_enabled = false;
			_IsEnemyWeaponFired = false;
		}
		else
		{
			entity->m_sprite.setPosition(x, y);
		}

	}
}

void Game::HandleEnemyWeaponFiring()
{
	if (_IsEnemyWeaponFired == true)
		return;

	std::vector<std::shared_ptr<Entity>>::reverse_iterator rit = EntityManager::m_Entities.rbegin();
	for (; rit != EntityManager::m_Entities.rend(); rit++)
	{
		std::shared_ptr<Entity> entity = *rit;

		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::enemy)
		{
			continue;
		}

		// a little random...
		int r = rand() % 20;
		if (r != 10)
			continue;

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;
		x--;

		std::shared_ptr<Entity> sw = std::make_shared<Entity>();
		sw->m_sprite.setTexture(_TextureWeaponEnemy);
		sw->m_sprite.setPosition(
			x + _TextureWeaponEnemy.getSize().x / 2,
			y + _TextureWeaponEnemy.getSize().y);

		sw->m_sprite.setPosition(
			entity->m_sprite.getPosition().x + _TextureEnemy.getSize().x / 2,
			entity->m_sprite.getPosition().y - 10);

		sw->m_type = EntityType::enemyWeapon;
		sw->m_size = _TextureWeaponEnemy.getSize();
		EntityManager::m_Entities.push_back(sw);

		// a little random...
		int random = ((double)rand() / (RAND_MAX)) + 1;
		if (random == 1) {
			_IsEnemyWeaponFired = false;
			_IsEnemyWeaponFired = true;
		}
		else {
			_IsEnemyWeaponFired = true;
		}

		break;
	}
}

void Game::HandleCollisionBlockEnemy()
{
	// Handle collision ennemy blocks

	for (std::shared_ptr<Entity> enemy : EntityManager::m_Entities)
	{
		if (enemy->m_enabled == false)
		{
			continue;
		}

		if (enemy->m_type != EntityType::enemy)
		{
			continue;
		}

		for (std::shared_ptr<Entity> block : EntityManager::m_Entities)
		{
			if (block->m_type != EntityType::block)
			{
				continue;
			}

			if (block->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundEnemy;
			boundEnemy = enemy->m_sprite.getGlobalBounds();

			sf::FloatRect boundBlock;
			boundBlock = block->m_sprite.getGlobalBounds();

			if (boundEnemy.intersects(boundBlock) == true)
			{
				EntityManager::GetPlayer()->m_enabled = false;
				goto end3;
			}
		}
	}

end3:
	//nop
	return;
}

void Game::HandleCollisionPlayerBlocks()
{
	// Handle collision ennemy blocks

	for (std::shared_ptr<Entity> mPlayer : EntityManager::m_Entities)
	{
		if (mPlayer->m_enabled == false)
		{
			continue;
		}

		if (mPlayer->m_type != EntityType::enemy)
		{
			continue;
		}

		for (std::shared_ptr<Entity> block : EntityManager::m_Entities)
		{
			if (block->m_type != EntityType::block)
			{
				continue;
			}

			if (block->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundPlayer;
			boundPlayer = EntityManager::GetPlayer()->m_sprite.getGlobalBounds();

			sf::FloatRect boundBlock;
			boundBlock = block->m_sprite.getGlobalBounds();

			if (boundPlayer.intersects(boundBlock) == true)
			{
				EntityManager::GetPlayer()->m_sprite.setPosition(EntityManager::GetPlayer()->m_sprite.getPosition().x - 10, EntityManager::GetPlayer()->m_sprite.getPosition().y);
				_lives--;
				goto end3;
			}
		}
	}

end3:
	//nop
	return;
}

void Game::HandleEnemyMoves()
{
	//
	// Handle Enemy moves
	//

	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::enemy)
		{
			continue;
		}

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;

		if (entity->m_bLeftToRight == true)
			x++;
		else
			x--;
		entity->m_times++;

		if (entity->m_times >= 150) //0)
		{
			if (entity->m_bLeftToRight == true)
			{
				entity->m_bLeftToRight = false;
				entity->m_times = 0;
			}
			else
			{
				entity->m_bLeftToRight = true;
				entity->m_times = 0;
				y += 1;
			}
		}

		entity->m_sprite.setPosition(x, y);
	}
}

void Game::HanldeWeaponMoves()
{
	//
	// Handle Weapon moves
	//

	for (std::shared_ptr<Entity> entity : EntityManager::m_Entities)
	{
		if (entity->m_enabled == false)
		{
			continue;
		}

		if (entity->m_type != EntityType::weapon)
		{
			continue;
		}

		float x, y;
		x = entity->m_sprite.getPosition().x;
		y = entity->m_sprite.getPosition().y;
		x++;

		if (x <= 0)
		{
			entity->m_enabled = false;
			_IsPlayerWeaponFired = false;
		}

		entity->m_sprite.setPosition(x, y);
	}
}

void Game::HandleCollisionWeaponBlock()
{
	// Handle collision weapon blocks

	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::weapon)
		{
			continue;
		}

		for (std::shared_ptr<Entity> block : EntityManager::m_Entities)
		{
			if (block->m_type != EntityType::block)
			{
				continue;
			}

			if (block->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundWeapon;
			boundWeapon = weapon->m_sprite.getGlobalBounds();

			sf::FloatRect boundBlock;
			boundBlock = block->m_sprite.getGlobalBounds();

			if (boundWeapon.intersects(boundBlock) == true)
			{
				weapon->m_enabled = false;
				_IsPlayerWeaponFired = false;
				//break;
				goto end2;
			}
		}
	}

end2:
	//nop
	return;
}

void Game::HandleCollisionWeaponEnemy()
{
	// Handle collision weapon enemies

	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::weapon)
		{
			continue;
		}

		for (std::shared_ptr<Entity> enemy : EntityManager::m_Entities)
		{
			if (enemy->m_type != EntityType::enemy)
			{
				continue;
			}

			if (enemy->m_enabled == false)
			{
				continue;
			}

			sf::FloatRect boundWeapon;
			boundWeapon = weapon->m_sprite.getGlobalBounds();

			sf::FloatRect boundEnemy;
			boundEnemy = enemy->m_sprite.getGlobalBounds();

			if (boundWeapon.intersects(boundEnemy) == true)
			{
				enemy->m_enabled = false;
				weapon->m_enabled = false;
				_IsPlayerWeaponFired = false;
				_score += 10;
				//break;
				goto end;
			}
		}
	}

end:
	//nop
	return;
}

void Game::HandleCollisionWeaponEnemyMaster()
{
	// Handle collision weapon master enemy

	for (std::shared_ptr<Entity> weapon : EntityManager::m_Entities)
	{
		if (weapon->m_enabled == false)
		{
			continue;
		}

		if (weapon->m_type != EntityType::weapon)
		{
			continue;
		}

		for (std::shared_ptr<Entity> enemy : EntityManager::m_Entities)
		{
			if (enemy->m_type != EntityType::enemyMaster)
			{
				continue;
			}

			if (enemy->m_enabled == false)
			{
				continue;
			}
			float x, y;
			x = enemy->m_sprite.getPosition().x;
			y = enemy->m_sprite.getPosition().y;

			sf::FloatRect boundWeapon;
			boundWeapon = weapon->m_sprite.getGlobalBounds();

			sf::FloatRect boundEnemy;
			boundEnemy = enemy->m_sprite.getGlobalBounds();

			if (boundWeapon.intersects(boundEnemy) == true)
			{
				_bosslife -= 50;

				if (_bosslife <= 0) {
					enemy->m_enabled = false;
					_score += 100;
				}

				if (_bosslife < 150) {
					enemy->m_sprite.setPosition(x - 200, y);
				}

				weapon->m_enabled = false;
				_IsPlayerWeaponFired = false;
				//break;
				goto end;

			}
		}
	}

end:
	//nop
	return;
}

void Game::HandleGameOver()
{
	// Game Over ?
	int count = std::count_if(EntityManager::m_Entities.begin(), EntityManager::m_Entities.end(), [](std::shared_ptr<Entity> element) {
		if (element->m_type == EntityType::enemy || element->m_type == EntityType::enemyMaster)
		{
			if (element->m_enabled == false)
			{
				return true;
			}
		}
		return false;
		});

	// sprite counts + enemy master
	//if (count >= (5))
	if (count == ((SPRITE_COUNT_X * SPRITE_COUNT_Y) + 1))
	{
		DisplayGameOver();
	}

	if (EntityManager::GetPlayer()->m_enabled == false)
	{
		DisplayGameOver();
	}

	if (_lives == 0)
	{
		DisplayGameOver();
	}
}

void Game::DisplayGameOver()
{
	if (_lives == 0)
	{
		mText.setFillColor(sf::Color::Green);
		mText.setFont(mFont);
		mText.setPosition(200.f, 200.f);
		mText.setCharacterSize(80);

		mText.setString("GAME OVER");

		_IsGameOver = true;
	}
	else
	{
		ResetSprites();
	}
}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed)
{
	if (key == sf::Keyboard::Up)
		mIsMovingUp = isPressed;
	else if (key == sf::Keyboard::Down)
		mIsMovingDown = isPressed;
	else if (key == sf::Keyboard::Left)
		mIsMovingLeft = isPressed;
	else if (key == sf::Keyboard::Right)
		mIsMovingRight = isPressed;

	if (key == sf::Keyboard::Space)
	{
		if (isPressed == false)
		{

			std::shared_ptr<Entity> sw = std::make_shared<Entity>();
			sw->m_sprite.setTexture(_TextureWeapon);
			sw->m_sprite.setPosition(
				EntityManager::GetPlayer()->m_sprite.getPosition().x + EntityManager::GetPlayer()->m_sprite.getTexture()->getSize().x,
				EntityManager::GetPlayer()->m_sprite.getPosition().y + 50);
			sw->m_type = EntityType::weapon;
			sw->m_size = _TextureWeapon.getSize();
			EntityManager::m_Entities.push_back(sw);

			_IsPlayerWeaponFired = true;

			return;
		}

		if (_IsPlayerWeaponFired == true)
		{
			return;
		}

	}
}
