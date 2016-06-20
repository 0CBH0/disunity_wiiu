#include "disunity.h"

u16 Low2Big_u16(u16 a);
u32 Low2Big_u32(u32 a);
u64 Low2Big_u64(u64 a);
void fread16_BE(u16 &i, FILE *f);
void fread32_BE(u32 &i, FILE *f);
void fread64_BE(u64 &i, FILE *f);
void fwrite16_BE(u16 i, FILE *f);
void fwrite32_BE(u32 i, FILE *f);
void fwrite64_BE(u64 i, FILE *f);
void intiFileType();
int fcopy(char *src_name, char *dest_name);
vector<ClassData>FileType;

int unpack(char *fileName)
{
	FILE *assets = fopen(fileName, "rb");
	char path[256];
	for(u32 i=0; i<strlen(fileName); i++)
	{
		if(fileName[i] == '.')
		{
			path[i] = '\0';
			break;
		}
		path[i] = fileName[i];
	}
	mkdir(path);
	intiFileType();
	u32 tableSize, dataEnd, fileGen, dataOffset, 
		platform, baseCount, assetCount;
	fread32_BE(tableSize, assets);
	fread32_BE(dataEnd, assets);
	fread32_BE(fileGen, assets);
	fread32_BE(dataOffset, assets);
	if(fileGen != 0x9) return -1;
	fread32_BE(tempa_u32, assets);
	string m_Version;
	while(tempa_u8=getc(assets)!=0)
		m_Version.push_back(tempa_u8);
	m_Version.push_back(tempa_u8);
	fread32_BE(platform, assets);
	if(platform != 0x1D && platform != 0x22) return -1;
	fread32_BE(baseCount, assets);
	if(baseCount != 0) return -1;
	fread32_BE(tempa_u32, assets);
	fread32_BE(assetCount, assets);
	AssetPreloadData *asset_table = new AssetPreloadData[assetCount];
	for(u32 i=0; i<assetCount; i++)
	{
		fread32_BE(asset_table[i].m_PathID, assets);
		fread32_BE(asset_table[i].Offset, assets);
		fread32_BE(asset_table[i].Size, assets);
		fread32_BE(asset_table[i].Type1, assets);
		fread32_BE(asset_table[i].Type2, assets);
		asset_table[i].Offset += dataOffset;
	}
	for(u32 i=0; i<assetCount; i++)
	{
		if(asset_table[i].Type1 == 0xFFFFFFFF || FileType[asset_table[i].Type1].info == -1) continue;
		fseek(assets, asset_table[i].Offset, 0);
		fread32_BE(tempa_u32, assets);
		for(u32 j=0; j<tempa_u32; j++)
			asset_table[i].name[j] = getc(assets);
		asset_table[i].name[tempa_u32] = '\0';
		if(tempa_u32 == 0 || asset_table[i].name[0] == 0) continue;
		while(ftell(assets)%4 != 0) getc(assets);
		char tempName[256];
		sprintf(tempName, "%s\\%s\\%s", path, FileType[asset_table[i].Type1].name.c_str(), asset_table[i].name);
		printf("%s", tempName);
		fseek(assets, FileType[asset_table[i].Type1].info, 1);
		fread32_BE(tempa_u32, assets);
		u32 data_size = tempa_u32;
		if(data_size >= asset_table[i].Size-4)
		{
			printf(" - worry\n");
			continue;
		}
		printf("\n");
		char tempPath[256];
		sprintf(tempPath, "%s\\%s", path, FileType[asset_table[i].Type1].name.c_str());
		mkdir(tempPath);
		FILE *file = fopen(tempName, "wb");
		u32 block_size = 512;
		while(data_size>0)
		{
			char data[512];
			block_size = 512;
			if(data_size<block_size)
				block_size = data_size;
			data_size -= block_size;
			fread(data, 1, block_size, assets);
			fwrite(data, 1, block_size, file);
		}
		fclose(file);
	}
	delete[]asset_table;
	fclose(assets);
	return 0;
}

int pack(char *fileName)
{
	FILE *assets = fopen(fileName, "rb");
	char path[256];
	for(u32 i=0; i<strlen(fileName); i++)
	{
		if(fileName[i] == '.')
		{
			path[i] = '\0';
			break;
		}
		path[i] = fileName[i];
	}
	if(access(path, 0) == -1) return 0;
	intiFileType();
	FILE *temp_assets = fopen("temp.bin", "wb");
	u32 tableSize, dataEnd, fileGen, dataOffset, 
		platform, baseCount, assetCount;
	fread32_BE(tableSize, assets);
	fread32_BE(dataEnd, assets);
	fread32_BE(fileGen, assets);
	fread32_BE(dataOffset, assets);
	if(fileGen != 0x9) return -1;
	fread32_BE(tempa_u32, assets);
	string m_Version;
	while(tempa_u8=getc(assets)!=0)
		m_Version.push_back(tempa_u8);
	m_Version.push_back(tempa_u8);
	fread32_BE(platform, assets);
	if(platform != 0x1D && platform != 0x22) return -1;
	fread32_BE(baseCount, assets);
	if(baseCount != 0) return -1;
	fread32_BE(tempa_u32, assets);
	fread32_BE(assetCount, assets);
	AssetPreloadData *asset_table = new AssetPreloadData[assetCount];
	ulg asset_table_ptr = ftell(assets);
	for(u32 i=0; i<assetCount; i++)
	{
		fread32_BE(asset_table[i].m_PathID, assets);
		fread32_BE(asset_table[i].Offset, assets);
		fread32_BE(asset_table[i].Size, assets);
		fread32_BE(asset_table[i].Type1, assets);
		fread32_BE(asset_table[i].Type2, assets);
		asset_table[i].Offset += dataOffset;
	}
	rewind(assets);
	while(ftell(assets) < dataOffset) putc(getc(assets), temp_assets);
	for(u32 i=0; i<assetCount; i++)
	{
		asset_table[i].NewOffset = ftell(temp_assets);
		fseek(assets, asset_table[i].Offset, 0);
		if(asset_table[i].Type1 == 0xFFFFFFFF || FileType[asset_table[i].Type1].info == -1)
		{
			// unknown type
			u32 data_size = asset_table[i].Size;
			u32 block_size = 512;
			while(data_size>0)
			{
				char data[512];
				block_size = 512;
				if(data_size<block_size)
					block_size = data_size;
				data_size -= block_size;
				fread(data, 1, block_size, assets);
				fwrite(data, 1, block_size, temp_assets);
			}
			asset_table[i].NewSize = asset_table[i].Size;
			continue;
		}
		fread32_BE(tempa_u32, assets);
		fwrite32_BE(tempa_u32, temp_assets);
		for(u32 j=0; j<tempa_u32; j++)
		{
			asset_table[i].name[j] = getc(assets);
			putc(asset_table[i].name[j], temp_assets);
		}
		asset_table[i].name[tempa_u32] = '\0';
		if(tempa_u32 == 0 || asset_table[i].name[0] == 0)
		{
			// empty name
			u32 data_size = asset_table[i].Size-4;
			u32 block_size = 512;
			while(data_size>0)
			{
				char data[512];
				block_size = 512;
				if(data_size<block_size)
					block_size = data_size;
				data_size -= block_size;
				fread(data, 1, block_size, assets);
				fwrite(data, 1, block_size, temp_assets);
			}
			asset_table[i].NewSize = asset_table[i].Size;
			continue;
		}
		while(ftell(temp_assets)%4 != 0) putc(0, temp_assets);
		char tempName[256];
		sprintf(tempName, "%s\\%s\\%s", path, FileType[asset_table[i].Type1].name.c_str(), asset_table[i].name);
		printf("%s\n", tempName);
		for(u32 j=0; j<FileType[asset_table[i].Type1].info; j++)
			putc(getc(assets), temp_assets);
		if(access(tempName, 0) == -1)
		{
			// lost file
			fseek(assets, asset_table[i].Offset, 0);
			fseek(temp_assets, asset_table[i].NewOffset, 0);
			u32 data_size = asset_table[i].Size;
			u32 block_size = 512;
			while(data_size>0)
			{
				char data[512];
				block_size = 512;
				if(data_size<block_size)
					block_size = data_size;
				data_size -= block_size;
				fread(data, 1, block_size, assets);
				fwrite(data, 1, block_size, temp_assets);
			}
			asset_table[i].NewSize = asset_table[i].Size;
			continue;
		}
		FILE *file = fopen(tempName, "rb");
		fseek(file, 0, 2);
		u32 data_size = ftell(file);
		rewind(file);
		fwrite32_BE(data_size, temp_assets);
		u32 block_size = 512;
		while(data_size>0)
		{
			char data[512];
			block_size = 512;
			if(data_size<block_size)
				block_size = data_size;
			data_size -= block_size;
			fread(data, 1, block_size, file);
			fwrite(data, 1, block_size, temp_assets);
		}
		fclose(file);
		while(ftell(temp_assets)%8 != 0) putc(0, temp_assets);
		fwrite32_BE(0, temp_assets);
		fwrite32_BE(0, temp_assets);
		asset_table[i].NewSize = ftell(temp_assets)-asset_table[i].NewOffset;
	}
	fseek(temp_assets, asset_table_ptr, 0);
	for(u32 i=0; i<assetCount; i++)
	{
		asset_table[i].NewOffset -= dataOffset;
		fwrite32_BE(asset_table[i].m_PathID, temp_assets);
		fwrite32_BE(asset_table[i].NewOffset, temp_assets);
		fwrite32_BE(asset_table[i].NewSize, temp_assets);
		fwrite32_BE(asset_table[i].Type1, temp_assets);
		fwrite32_BE(asset_table[i].Type2, temp_assets);
	}
	delete[]asset_table;
	fclose(assets);
	fclose(temp_assets);
	fcopy("temp.bin", fileName);
	remove("temp.bin");
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Usage:\n");
		printf("unpack: disunity_wiiu -d [file_name]\n");
		printf("pack: disunity_wiiu -p [file_name]\n");
		return 0;
	}
	if(strcmp("-d", argv[1]) == 0) unpack(argv[2]);
	if(strcmp("-p", argv[1]) == 0) pack(argv[2]);
	return 0;
}

void intiFileType()
{
	ClassData temp;
	temp.info = 0;
	temp.name = "";
	for(u32 i=0; i<1200; i++)
		FileType.push_back(temp);
	FileType[1].name = "GameObject";
	FileType[2].name = "Component";
	FileType[3].name = "LevelGameManager";
	FileType[4].name = "Transform";
	FileType[5].name = "TimeManager";
	FileType[6].name = "GlobalGameManager";
	FileType[8].name = "Behaviour";
	FileType[9].name = "GameManager";
	FileType[11].name = "AudioManager";
	FileType[12].name = "ParticleAnimator";
	FileType[13].name = "InputManager";
	FileType[15].name = "EllipsoidParticleEmitter";
	FileType[17].name = "Pipeline";
	FileType[18].name = "EditorExtension";
	FileType[19].name = "Physics2DSettings";
	FileType[20].name = "Camera";
	FileType[21].name = "Material";
	FileType[21].info = -1;
	FileType[23].name = "MeshRenderer";
	FileType[25].name = "Renderer";
	FileType[26].name = "ParticleRenderer";
	FileType[27].name = "Texture";
	FileType[28].name = "Texture2D";
	FileType[28].info = 0x34;
	FileType[29].name = "SceneSettings";
	FileType[30].name = "GraphicsSettings";
	FileType[33].name = "MeshFilter";
	FileType[41].name = "OcclusionPortal";
	FileType[43].name = "Mesh";
	FileType[45].name = "Skybox";
	FileType[47].name = "QualitySettings";
	FileType[48].name = "Shader";
	FileType[49].name = "TextAsset";
	FileType[50].name = "Rigidbody2D";
	FileType[51].name = "Physics2DManager";
	FileType[53].name = "Collider2D";
	FileType[54].name = "Rigidbody";
	FileType[55].name = "PhysicsManager";
	FileType[56].name = "Collider";
	FileType[57].name = "Joint";
	FileType[58].name = "CircleCollider2D";
	FileType[59].name = "HingeJoint";
	FileType[60].name = "PolygonCollider2D";
	FileType[61].name = "BoxCollider2D";
	FileType[62].name = "PhysicsMaterial2D";
	FileType[64].name = "MeshCollider";
	FileType[65].name = "BoxCollider";
	FileType[66].name = "SpriteCollider2D";
	FileType[68].name = "EdgeCollider2D";
	FileType[72].name = "ComputeShader";
	FileType[74].name = "AnimationClip";
	FileType[75].name = "ConstantForce";
	FileType[76].name = "WorldParticleCollider";
	FileType[78].name = "TagManager";
	FileType[81].name = "AudioListener";
	FileType[82].name = "AudioSource";
	FileType[83].name = "AudioClip";
	FileType[83].info = 0x10;
	FileType[84].name = "RenderTexture";
	FileType[87].name = "MeshParticleEmitter";
	FileType[88].name = "ParticleEmitter";
	FileType[89].name = "Cubemap";
	FileType[90].name = "Avatar";
	FileType[91].name = "AnimatorController";
	FileType[92].name = "GUILayer";
	FileType[93].name = "RuntimeAnimatorController";
	FileType[94].name = "ScriptMapper";
	FileType[95].name = "Animator";
	FileType[96].name = "TrailRenderer";
	FileType[98].name = "DelayedCallManager";
	FileType[102].name = "TextMesh";
	FileType[104].name = "RenderSettings";
	FileType[108].name = "Light";
	FileType[109].name = "CGProgram";
	FileType[110].name = "BaseAnimationTrack";
	FileType[111].name = "Animation";
	FileType[114].name = "MonoBehaviour";
	FileType[115].name = "MonoScript";
	FileType[116].name = "MonoManager";
	FileType[117].name = "Texture3D";
	FileType[118].name = "NewAnimationTrack";
	FileType[119].name = "Projector";
	FileType[120].name = "LineRenderer";
	FileType[121].name = "Flare";
	FileType[122].name = "Halo";
	FileType[123].name = "LensFlare";
	FileType[124].name = "FlareLayer";
	FileType[125].name = "HaloLayer";
	FileType[126].name = "NavMeshAreas";
	FileType[127].name = "HaloManager";
	FileType[128].name = "Font";
	FileType[128].info = 0x34;
	FileType[129].name = "PlayerSettings";
	FileType[130].name = "NamedObject";
	FileType[131].name = "GUITexture";
	FileType[132].name = "GUIText";
	FileType[133].name = "GUIElement";
	FileType[134].name = "PhysicMaterial";
	FileType[135].name = "SphereCollider";
	FileType[136].name = "CapsuleCollider";
	FileType[137].name = "SkinnedMeshRenderer";
	FileType[138].name = "FixedJoint";
	FileType[140].name = "RaycastCollider";
	FileType[141].name = "BuildSettings";
	FileType[142].name = "AssetBundle";
	FileType[143].name = "CharacterController";
	FileType[144].name = "CharacterJoint";
	FileType[145].name = "SpringJoint";
	FileType[146].name = "WheelCollider";
	FileType[147].name = "ResourceManager";
	FileType[148].name = "NetworkView";
	FileType[149].name = "NetworkManager";
	FileType[150].name = "PreloadData";
	FileType[152].name = "MovieTexture";
	FileType[153].name = "ConfigurableJoint";
	FileType[154].name = "TerrainCollider";
	FileType[155].name = "MasterServerInterface";
	FileType[156].name = "TerrainData";
	FileType[157].name = "LightmapSettings";
	FileType[158].name = "WebCamTexture";
	FileType[159].name = "EditorSettings";
	FileType[160].name = "InteractiveCloth";
	FileType[161].name = "ClothRenderer";
	FileType[162].name = "EditorUserSettings";
	FileType[163].name = "SkinnedCloth";
	FileType[164].name = "AudioReverbFilter";
	FileType[165].name = "AudioHighPassFilter";
	FileType[166].name = "AudioChorusFilter";
	FileType[167].name = "AudioReverbZone";
	FileType[168].name = "AudioEchoFilter";
	FileType[169].name = "AudioLowPassFilter";
	FileType[170].name = "AudioDistortionFilter";
	FileType[171].name = "SparseTexture";
	FileType[180].name = "AudioBehaviour";
	FileType[181].name = "AudioFilter";
	FileType[182].name = "WindZone";
	FileType[183].name = "Cloth";
	FileType[184].name = "SubstanceArchive";
	FileType[185].name = "ProceduralMaterial";
	FileType[186].name = "ProceduralTexture";
	FileType[191].name = "OffMeshLink";
	FileType[192].name = "OcclusionArea";
	FileType[193].name = "Tree";
	FileType[194].name = "NavMeshObsolete";
	FileType[195].name = "NavMeshAgent";
	FileType[196].name = "NavMeshSettings";
	FileType[197].name = "LightProbesLegacy";
	FileType[198].name = "ParticleSystem";
	FileType[199].name = "ParticleSystemRenderer";
	FileType[200].name = "ShaderVariantCollection";
	FileType[205].name = "LODGroup";
	FileType[206].name = "BlendTree";
	FileType[207].name = "Motion";
	FileType[208].name = "NavMeshObstacle";
	FileType[210].name = "TerrainInstance";
	FileType[212].name = "SpriteRenderer";
	FileType[213].name = "Sprite";
	FileType[214].name = "CachedSpriteAtlas";
	FileType[215].name = "ReflectionProbe";
	FileType[216].name = "ReflectionProbes";
	FileType[220].name = "LightProbeGroup";
	FileType[221].name = "AnimatorOverrideController";
	FileType[222].name = "CanvasRenderer";
	FileType[223].name = "Canvas";
	FileType[224].name = "RectTransform";
	FileType[225].name = "CanvasGroup";
	FileType[226].name = "BillboardAsset";
	FileType[227].name = "BillboardRenderer";
	FileType[228].name = "SpeedTreeWindAsset";
	FileType[229].name = "AnchoredJoint2D";
	FileType[230].name = "Joint2D";
	FileType[231].name = "SpringJoint2D";
	FileType[232].name = "DistanceJoint2D";
	FileType[233].name = "HingeJoint2D";
	FileType[234].name = "SliderJoint2D";
	FileType[235].name = "WheelJoint2D";
	FileType[238].name = "NavMeshData";
	FileType[240].name = "AudioMixer";
	FileType[241].name = "AudioMixerController";
	FileType[243].name = "AudioMixerGroupController";
	FileType[244].name = "AudioMixerEffectController";
	FileType[245].name = "AudioMixerSnapshotController";
	FileType[246].name = "PhysicsUpdateBehaviour2D";
	FileType[247].name = "ConstantForce2D";
	FileType[248].name = "Effector2D";
	FileType[249].name = "AreaEffector2D";
	FileType[250].name = "PointEffector2D";
	FileType[251].name = "PlatformEffector2D";
	FileType[252].name = "SurfaceEffector2D";
	FileType[258].name = "LightProbes";
	FileType[271].name = "SampleClip";
	FileType[272].name = "AudioMixerSnapshot";
	FileType[273].name = "AudioMixerGroup";
	FileType[290].name = "AssetBundleManifest";
	FileType[1001].name = "Prefab";
	FileType[1002].name = "EditorExtensionImpl";
	FileType[1003].name = "AssetImporter";
	FileType[1004].name = "AssetDatabase";
	FileType[1005].name = "Mesh3DSImporter";
	FileType[1006].name = "TextureImporter";
	FileType[1007].name = "ShaderImporter";
	FileType[1008].name = "ComputeShaderImporter";
	FileType[1011].name = "AvatarMask";
	FileType[1020].name = "AudioImporter";
	FileType[1026].name = "HierarchyState";
	FileType[1027].name = "GUIDSerializer";
	FileType[1028].name = "AssetMetaData";
	FileType[1029].name = "DefaultAsset";
	FileType[1030].name = "DefaultImporter";
	FileType[1031].name = "TextScriptImporter";
	FileType[1032].name = "SceneAsset";
	FileType[1034].name = "NativeFormatImporter";
	FileType[1035].name = "MonoImporter";
	FileType[1037].name = "AssetServerCache";
	FileType[1038].name = "LibraryAssetImporter";
	FileType[1040].name = "ModelImporter";
	FileType[1041].name = "FBXImporter";
	FileType[1042].name = "TrueTypeFontImporter";
	FileType[1044].name = "MovieImporter";
	FileType[1045].name = "EditorBuildSettings";
	FileType[1046].name = "DDSImporter";
	FileType[1048].name = "InspectorExpandedState";
	FileType[1049].name = "AnnotationManager";
	FileType[1050].name = "PluginImporter";
	FileType[1051].name = "EditorUserBuildSettings";
	FileType[1052].name = "PVRImporter";
	FileType[1053].name = "ASTCImporter";
	FileType[1054].name = "KTXImporter";
	FileType[1101].name = "AnimatorStateTransition";
	FileType[1102].name = "AnimatorState";
	FileType[1105].name = "HumanTemplate";
	FileType[1107].name = "AnimatorStateMachine";
	FileType[1108].name = "PreviewAssetType";
	FileType[1109].name = "AnimatorTransition";
	FileType[1110].name = "SpeedTreeImporter";
	FileType[1111].name = "AnimatorTransitionBase";
	FileType[1112].name = "SubstanceImporter";
	FileType[1113].name = "LightmapParameters";
	FileType[1120].name = "LightmapSnapshot";
	for(u32 i=0; i<1200; i++)
		if(FileType[i].name.length()<2)
			FileType[i].info = -1;
}

u16 Low2Big_u16(u16 a)
{
	u16 b=0;
	b=(a&0xFF00)>>8|(a&0xFF)<<8;
	return b;
}

u32 Low2Big_u32(u32 a)
{
	u32 b=0;
	b=(a&0xFF000000)>>24|(a&0xFF0000)>>8|(a&0xFF00)<<8|(a&0xFF)<<24;
	return b;
}

u64 Low2Big_u64(u64 a)
{
	u64 b=0;
	b = (a&0xFF)<<56|(a&0xFF00)<<40|(a&0xFF0000)<<24|(a&0xFF000000)<<8|(a&0xFF00000000)>>8|
		(a&0xFF0000000000)>>24|(a&0xFF000000000000)>>40|(a&0xFF00000000000000)>>56;
	return b;
}

void fread16_BE(u16 &i, FILE *f)
{
	u16 p = 0;
	fread(&p, sizeof(u16), 1, f);
	i = Low2Big_u16(p);
}

void fread32_BE(u32 &i, FILE *f)
{
	u32 p = 0;
	fread(&p, sizeof(u32), 1, f);
	i = Low2Big_u32(p);
}

void fread64_BE(u64 &i, FILE *f)
{
	u64 p = 0;
	fread(&p, sizeof(u64), 1, f);
	i = Low2Big_u64(p);
}

void fwrite16_BE(u16 i, FILE *f)
{
	u16 p = Low2Big_u16(i);
	fwrite(&p, sizeof(u16), 1, f);
}

void fwrite32_BE(u32 i, FILE *f)
{
	u32 p = Low2Big_u32(i);
	fwrite(&p, sizeof(u32), 1, f);
}

void fwrite64_BE(u64 i, FILE *f)
{
	u64 p = Low2Big_u64(i);
	fwrite(&p, sizeof(u64), 1, f);
}

int fcopy(char *src_name, char *dest_name)
{
	FILE *src=fopen(src_name, "rb");
	if(src == NULL) return -1;
	FILE *dest=fopen(dest_name, "wb");
	fseek(src, 0, 2);
	unsigned int data_size=ftell(src);
	rewind(src);
	unsigned int block_size = 512;
	while(data_size>0)
	{
		char data[512];
		block_size = 512;
		if(data_size<block_size)
			block_size = data_size;
		data_size -= block_size;
		fread(data, 1, block_size, src);
		fwrite(data, 1, block_size, dest);
	}
	fclose(src);
	fclose(dest);
	return 0;
}
