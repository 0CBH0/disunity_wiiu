#include <io.h>
#include <string.h>
#include <direct.h>
#include <vector>
#include "disunity.h"

#pragma warning(disable : 4996)

using namespace std;

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
vector<string>FileType;

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
	if(platform != 0x1D) return -1;
	fread32_BE(baseCount, assets);
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
		char tempPath[256];
		if(FileType[asset_table[i].Type1].length()>1)
			sprintf(tempPath, "%s\\%s", path, FileType[asset_table[i].Type1].c_str());
		else
			sprintf(tempPath, "%s\\unknown", path);
		mkdir(tempPath);
	}
	for(u32 i=0; i<assetCount; i++)
	{
		fseek(assets, asset_table[i].Offset, 0);
		fread32_BE(tempa_u32, assets);
		for(u32 j=0; j<tempa_u32; j++)
			asset_table[i].name[j] = getc(assets);
		asset_table[i].name[tempa_u32] = '\0';
		while(ftell(assets)%4 != 0) getc(assets);
		char tempName[256];
		if(FileType[asset_table[i].Type1].length()>1)
			sprintf(tempName, "%s\\%s\\%s.%s", path, FileType[asset_table[i].Type1].c_str(), asset_table[i].name, FileType[asset_table[i].Type1].c_str());
		else
			sprintf(tempName, "%s\\unknown\\%s", path, asset_table[i].name);
		printf("%s\n", tempName);
		FILE *file = fopen(tempName, "wb");
		fread32_BE(tempa_u32, assets);
		u32 data_size = tempa_u32;
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
	mkdir(path);
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
	if(platform != 0x1D) return -1;
	fread32_BE(baseCount, assets);
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
		fread32_BE(tempa_u32, assets);
		fwrite32_BE(tempa_u32, temp_assets);
		for(u32 j=0; j<tempa_u32; j++)
		{
			asset_table[i].name[j] = getc(assets);
			putc(asset_table[i].name[j], temp_assets);
		}
		asset_table[i].name[tempa_u32] = '\0';
		while(ftell(assets)%4 != 0) getc(assets);
		while(ftell(temp_assets)%4 != 0) putc(0, temp_assets);
		char tempName[256];
		if(FileType[asset_table[i].Type1].length()>1)
			sprintf(tempName, "%s\\%s\\%s.%s", path, FileType[asset_table[i].Type1].c_str(), asset_table[i].name, FileType[asset_table[i].Type1].c_str());
		else
			sprintf(tempName, "%s\\unknown\\%s.bin", path, asset_table[i].name);
		printf("%s\n", tempName);
		if(!access(tempName, 0))
		{
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
		}
		else
		{
			printf("here");
			fread32_BE(tempa_u32, assets);
			u32 data_size = tempa_u32;
			fwrite32_BE(data_size, temp_assets);
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
		}
		while(ftell(temp_assets)%8 != 0) putc(0, temp_assets);
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
	for(u32 i=0; i<1200; i++)
		FileType.push_back("");
	FileType[1] = "GameObject";
	FileType[2] = "Component";
	FileType[3] = "LevelGameManager";
	FileType[4] = "Transform";
	FileType[5] = "TimeManager";
	FileType[6] = "GlobalGameManager";
	FileType[8] = "Behaviour";
	FileType[9] = "GameManager";
	FileType[11] = "AudioManager";
	FileType[12] = "ParticleAnimator";
	FileType[13] = "InputManager";
	FileType[15] = "EllipsoidParticleEmitter";
	FileType[17] = "Pipeline";
	FileType[18] = "EditorExtension";
	FileType[19] = "Physics2DSettings";
	FileType[20] = "Camera";
	FileType[21] = "Material";
	FileType[23] = "MeshRenderer";
	FileType[25] = "Renderer";
	FileType[26] = "ParticleRenderer";
	FileType[27] = "Texture";
	FileType[28] = "Texture2D";
	FileType[29] = "SceneSettings";
	FileType[30] = "GraphicsSettings";
	FileType[33] = "MeshFilter";
	FileType[41] = "OcclusionPortal";
	FileType[43] = "Mesh";
	FileType[45] = "Skybox";
	FileType[47] = "QualitySettings";
	FileType[48] = "Shader";
	FileType[49] = "TextAsset";
	FileType[50] = "Rigidbody2D";
	FileType[51] = "Physics2DManager";
	FileType[53] = "Collider2D";
	FileType[54] = "Rigidbody";
	FileType[55] = "PhysicsManager";
	FileType[56] = "Collider";
	FileType[57] = "Joint";
	FileType[58] = "CircleCollider2D";
	FileType[59] = "HingeJoint";
	FileType[60] = "PolygonCollider2D";
	FileType[61] = "BoxCollider2D";
	FileType[62] = "PhysicsMaterial2D";
	FileType[64] = "MeshCollider";
	FileType[65] = "BoxCollider";
	FileType[66] = "SpriteCollider2D";
	FileType[68] = "EdgeCollider2D";
	FileType[72] = "ComputeShader";
	FileType[74] = "AnimationClip";
	FileType[75] = "ConstantForce";
	FileType[76] = "WorldParticleCollider";
	FileType[78] = "TagManager";
	FileType[81] = "AudioListener";
	FileType[82] = "AudioSource";
	FileType[83] = "AudioClip";
	FileType[84] = "RenderTexture";
	FileType[87] = "MeshParticleEmitter";
	FileType[88] = "ParticleEmitter";
	FileType[89] = "Cubemap";
	FileType[90] = "Avatar";
	FileType[91] = "AnimatorController";
	FileType[92] = "GUILayer";
	FileType[93] = "RuntimeAnimatorController";
	FileType[94] = "ScriptMapper";
	FileType[95] = "Animator";
	FileType[96] = "TrailRenderer";
	FileType[98] = "DelayedCallManager";
	FileType[102] = "TextMesh";
	FileType[104] = "RenderSettings";
	FileType[108] = "Light";
	FileType[109] = "CGProgram";
	FileType[110] = "BaseAnimationTrack";
	FileType[111] = "Animation";
	FileType[114] = "MonoBehaviour";
	FileType[115] = "MonoScript";
	FileType[116] = "MonoManager";
	FileType[117] = "Texture3D";
	FileType[118] = "NewAnimationTrack";
	FileType[119] = "Projector";
	FileType[120] = "LineRenderer";
	FileType[121] = "Flare";
	FileType[122] = "Halo";
	FileType[123] = "LensFlare";
	FileType[124] = "FlareLayer";
	FileType[125] = "HaloLayer";
	FileType[126] = "NavMeshAreas";
	FileType[127] = "HaloManager";
	FileType[128] = "Font";
	FileType[129] = "PlayerSettings";
	FileType[130] = "NamedObject";
	FileType[131] = "GUITexture";
	FileType[132] = "GUIText";
	FileType[133] = "GUIElement";
	FileType[134] = "PhysicMaterial";
	FileType[135] = "SphereCollider";
	FileType[136] = "CapsuleCollider";
	FileType[137] = "SkinnedMeshRenderer";
	FileType[138] = "FixedJoint";
	FileType[140] = "RaycastCollider";
	FileType[141] = "BuildSettings";
	FileType[142] = "AssetBundle";
	FileType[143] = "CharacterController";
	FileType[144] = "CharacterJoint";
	FileType[145] = "SpringJoint";
	FileType[146] = "WheelCollider";
	FileType[147] = "ResourceManager";
	FileType[148] = "NetworkView";
	FileType[149] = "NetworkManager";
	FileType[150] = "PreloadData";
	FileType[152] = "MovieTexture";
	FileType[153] = "ConfigurableJoint";
	FileType[154] = "TerrainCollider";
	FileType[155] = "MasterServerInterface";
	FileType[156] = "TerrainData";
	FileType[157] = "LightmapSettings";
	FileType[158] = "WebCamTexture";
	FileType[159] = "EditorSettings";
	FileType[160] = "InteractiveCloth";
	FileType[161] = "ClothRenderer";
	FileType[162] = "EditorUserSettings";
	FileType[163] = "SkinnedCloth";
	FileType[164] = "AudioReverbFilter";
	FileType[165] = "AudioHighPassFilter";
	FileType[166] = "AudioChorusFilter";
	FileType[167] = "AudioReverbZone";
	FileType[168] = "AudioEchoFilter";
	FileType[169] = "AudioLowPassFilter";
	FileType[170] = "AudioDistortionFilter";
	FileType[171] = "SparseTexture";
	FileType[180] = "AudioBehaviour";
	FileType[181] = "AudioFilter";
	FileType[182] = "WindZone";
	FileType[183] = "Cloth";
	FileType[184] = "SubstanceArchive";
	FileType[185] = "ProceduralMaterial";
	FileType[186] = "ProceduralTexture";
	FileType[191] = "OffMeshLink";
	FileType[192] = "OcclusionArea";
	FileType[193] = "Tree";
	FileType[194] = "NavMeshObsolete";
	FileType[195] = "NavMeshAgent";
	FileType[196] = "NavMeshSettings";
	FileType[197] = "LightProbesLegacy";
	FileType[198] = "ParticleSystem";
	FileType[199] = "ParticleSystemRenderer";
	FileType[200] = "ShaderVariantCollection";
	FileType[205] = "LODGroup";
	FileType[206] = "BlendTree";
	FileType[207] = "Motion";
	FileType[208] = "NavMeshObstacle";
	FileType[210] = "TerrainInstance";
	FileType[212] = "SpriteRenderer";
	FileType[213] = "Sprite";
	FileType[214] = "CachedSpriteAtlas";
	FileType[215] = "ReflectionProbe";
	FileType[216] = "ReflectionProbes";
	FileType[220] = "LightProbeGroup";
	FileType[221] = "AnimatorOverrideController";
	FileType[222] = "CanvasRenderer";
	FileType[223] = "Canvas";
	FileType[224] = "RectTransform";
	FileType[225] = "CanvasGroup";
	FileType[226] = "BillboardAsset";
	FileType[227] = "BillboardRenderer";
	FileType[228] = "SpeedTreeWindAsset";
	FileType[229] = "AnchoredJoint2D";
	FileType[230] = "Joint2D";
	FileType[231] = "SpringJoint2D";
	FileType[232] = "DistanceJoint2D";
	FileType[233] = "HingeJoint2D";
	FileType[234] = "SliderJoint2D";
	FileType[235] = "WheelJoint2D";
	FileType[238] = "NavMeshData";
	FileType[240] = "AudioMixer";
	FileType[241] = "AudioMixerController";
	FileType[243] = "AudioMixerGroupController";
	FileType[244] = "AudioMixerEffectController";
	FileType[245] = "AudioMixerSnapshotController";
	FileType[246] = "PhysicsUpdateBehaviour2D";
	FileType[247] = "ConstantForce2D";
	FileType[248] = "Effector2D";
	FileType[249] = "AreaEffector2D";
	FileType[250] = "PointEffector2D";
	FileType[251] = "PlatformEffector2D";
	FileType[252] = "SurfaceEffector2D";
	FileType[258] = "LightProbes";
	FileType[271] = "SampleClip";
	FileType[272] = "AudioMixerSnapshot";
	FileType[273] = "AudioMixerGroup";
	FileType[290] = "AssetBundleManifest";
	FileType[1001] = "Prefab";
	FileType[1002] = "EditorExtensionImpl";
	FileType[1003] = "AssetImporter";
	FileType[1004] = "AssetDatabase";
	FileType[1005] = "Mesh3DSImporter";
	FileType[1006] = "TextureImporter";
	FileType[1007] = "ShaderImporter";
	FileType[1008] = "ComputeShaderImporter";
	FileType[1011] = "AvatarMask";
	FileType[1020] = "AudioImporter";
	FileType[1026] = "HierarchyState";
	FileType[1027] = "GUIDSerializer";
	FileType[1028] = "AssetMetaData";
	FileType[1029] = "DefaultAsset";
	FileType[1030] = "DefaultImporter";
	FileType[1031] = "TextScriptImporter";
	FileType[1032] = "SceneAsset";
	FileType[1034] = "NativeFormatImporter";
	FileType[1035] = "MonoImporter";
	FileType[1037] = "AssetServerCache";
	FileType[1038] = "LibraryAssetImporter";
	FileType[1040] = "ModelImporter";
	FileType[1041] = "FBXImporter";
	FileType[1042] = "TrueTypeFontImporter";
	FileType[1044] = "MovieImporter";
	FileType[1045] = "EditorBuildSettings";
	FileType[1046] = "DDSImporter";
	FileType[1048] = "InspectorExpandedState";
	FileType[1049] = "AnnotationManager";
	FileType[1050] = "PluginImporter";
	FileType[1051] = "EditorUserBuildSettings";
	FileType[1052] = "PVRImporter";
	FileType[1053] = "ASTCImporter";
	FileType[1054] = "KTXImporter";
	FileType[1101] = "AnimatorStateTransition";
	FileType[1102] = "AnimatorState";
	FileType[1105] = "HumanTemplate";
	FileType[1107] = "AnimatorStateMachine";
	FileType[1108] = "PreviewAssetType";
	FileType[1109] = "AnimatorTransition";
	FileType[1110] = "SpeedTreeImporter";
	FileType[1111] = "AnimatorTransitionBase";
	FileType[1112] = "SubstanceImporter";
	FileType[1113] = "LightmapParameters";
	FileType[1120] = "LightmapSnapshot";
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
