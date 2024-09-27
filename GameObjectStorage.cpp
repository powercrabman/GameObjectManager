#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>

using ObjectID = size_t;

class GameObject
{
public:
	GameObject()
	{
		mObjectID = ++sGameObjectID;
	}

	inline ObjectID GetObjectID() const
	{
		return mObjectID;
	}

	void Update()
	{
		/* ... */
	}

private:
	inline static ObjectID sGameObjectID = 0;
	ObjectID mObjectID = 0;
};

class ObjectManager
{
public:
	ObjectManager()
	{
		mGameObjectRepo.reserve(sReserveCapacity);
		mUpdateObjectRepo.reserve(sReserveCapacity);
	}
	virtual ~ObjectManager() {}

	/* 순회 */
	inline void UpdateGameObject()
	{
		for (size_t i = 0; i < mValidItemSize; ++i)
		{
			GameObject* obj = mUpdateObjectRepo[i];
			obj->Update();
		}
	}

	/* 삽입 */
	/* 템플릿 */
	inline GameObject* CreateGameObject(/* 가변인자 템플릿 */)
	{
		GameObjectRef obj = std::make_unique<GameObject>(/* 매개변수 */);
		GameObject* ptr = obj.get();

		//배열에 업데이트
		size_t backIndex = mUpdateObjectRepo.size();

		assert(backIndex >= mValidItemSize);

		if (backIndex == mValidItemSize)
		{
			mUpdateObjectRepo.push_back(ptr);
			++mValidItemSize;
		}
		else
		{
			mUpdateObjectRepo[mValidItemSize++] = ptr;
		}

		//해쉬에 업데이트
		mGameObjectRepo[ptr->GetObjectID()] = std::make_pair(mValidItemSize - 1, std::move(obj));

		return ptr;
	}

	/* 탐색 */
	GameObject* GetGameObject(ObjectID inID)
	{
		auto iter = mGameObjectRepo.find(inID);
		assert(iter != mGameObjectRepo.end());
		return (*iter).second.second.get();
	}

	/* 삭제 */
	void RemoveGameObject(ObjectID inID)
	{
		//삭제할 요소 찾기
		auto iter = mGameObjectRepo.find(inID);
		assert(iter != mGameObjectRepo.end());

		//배열의 인덱스
		size_t arrayIdx = (*iter).second.first;

		//만약 요소가 1개라면 early return
		if (mValidItemSize == 1)
		{
			mUpdateObjectRepo[0] = nullptr;

			//해쉬 속 기존 ItemA 요소 제거
			mGameObjectRepo.erase(inID);
			--mValidItemSize;

			return;
		}

		//ItemA와 ItemB를 찾은후 자리 변경, ItemA는 nullptr로 변경
		mUpdateObjectRepo[arrayIdx] = mUpdateObjectRepo[mValidItemSize-1];
		mUpdateObjectRepo[mValidItemSize - 1] = nullptr;
		--mValidItemSize;

		//해쉬 속 기존 ItemA 요소 제거
		mGameObjectRepo.erase(inID);

		//해쉬 속 ItemB의 배열 인덱스 업데이트
		size_t oldID = mUpdateObjectRepo[arrayIdx]->GetObjectID();
		mGameObjectRepo[oldID].first = arrayIdx;
	}

	/* 가비지 컬렉터 */
	void CleanGarbge()
	{
		mUpdateObjectRepo.erase(mUpdateObjectRepo.begin() + mValidItemSize, mUpdateObjectRepo.end());
	}

private:
	using GameObjectRef = std::unique_ptr<GameObject>;
	constexpr inline static size_t sReserveCapacity = 1024;

	// 게임 오브젝트의 소유권
	// 키 : 게임오브젝트의 ID           값 : 배열의 인덱스, GameObject의 unique_ptr
	std::unordered_map<ObjectID, std::pair<size_t, GameObjectRef>> mGameObjectRepo;
	std::vector<GameObject*> mUpdateObjectRepo;
	size_t mValidItemSize = 0;
};

int main() {
	// ObjectManager 인스턴스 생성
	ObjectManager manager;

	// 3개의 GameObject 생성
	GameObject* obj1 = manager.CreateGameObject();
	GameObject* obj2 = manager.CreateGameObject();
	GameObject* obj3 = manager.CreateGameObject();

	// 각 객체의 ID 확인
	std::cout << "Object 1 ID: " << obj1->GetObjectID() << std::endl;
	std::cout << "Object 2 ID: " << obj2->GetObjectID() << std::endl;
	std::cout << "Object 3 ID: " << obj3->GetObjectID() << std::endl;

	// Update 테스트
	std::cout << "Updating all objects..." << std::endl;
	manager.UpdateGameObject();

	// 객체 삭제
	std::cout << "Removing Object 2..." << std::endl;
	manager.RemoveGameObject(obj2->GetObjectID());

	// 다시 순회
	std::cout << "Updating all objects after removal..." << std::endl;
	manager.UpdateGameObject();

	// 유효성 확인
	GameObject* findObj1 = manager.GetGameObject(obj1->GetObjectID());
	if (findObj1) {
		std::cout << "Object 1 still exists." << std::endl;
	}

	GameObject* findObj3 = manager.GetGameObject(obj3->GetObjectID());
	if (findObj3) {
		std::cout << "Object 3 still exists." << std::endl;
	}

	// 가비지 컬렉터 호출
	std::cout << "Cleaning up garbage..." << std::endl;
	manager.CleanGarbge();

	// 마지막 Update 테스트
	std::cout << "Final update after cleaning..." << std::endl;
	manager.UpdateGameObject();

	return 0;
}