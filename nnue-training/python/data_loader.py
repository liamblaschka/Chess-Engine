import ctypes
import os
import platform
import numpy as np
import torch

HALF_KP_SIZE = 40960
HALF_RELATIVE_KP_SIZE = 2250
KING_FACTOR_SIZE = 64

def _load_shared_library():
    system_name = platform.system()
    if system_name == "Darwin":
        lib_name = "libdataloader.dylib"
    elif system_name == "Linux":
        lib_name = "libdataloader.so"
    elif system_name == "Windows":
        lib_name = "libdataloader.dll"
    else:
        raise OSError(f"Unsupported operating system: {system_name}")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(script_dir)
    lib_path = os.path.join(project_dir, "build", lib_name)
    if not os.path.exists(lib_path):
        raise FileNotFoundError(f"Missing compiled binary at: {lib_path}\n")
    
    return ctypes.CDLL(lib_path)

_lib = _load_shared_library()


# Dataset
_lib.Dataset_new.argtypes = [ctypes.c_char_p]
_lib.Dataset_new.restype = ctypes.c_void_p

_lib.Dataset_delete.argtypes = [ctypes.c_void_p]
_lib.Dataset_delete.restype = None

_lib.Dataset_getDataSize.argtypes = [ctypes.c_void_p]
_lib.Dataset_getDataSize.restype = ctypes.c_int

# SparseFeatures
_lib.SparseFeatures_getWhiteIndices.argtypes = [ctypes.c_void_p]
_lib.SparseFeatures_getWhiteIndices.restype = ctypes.POINTER(ctypes.c_int)

_lib.SparseFeatures_getBlackIndices.argtypes = [ctypes.c_void_p]
_lib.SparseFeatures_getBlackIndices.restype = ctypes.POINTER(ctypes.c_int)

_lib.SparseFeatures_getNumActiveWhite.argtypes = [ctypes.c_void_p]
_lib.SparseFeatures_getNumActiveWhite.restype = ctypes.c_int

_lib.SparseFeatures_getNumActiveBlack.argtypes = [ctypes.c_void_p]
_lib.SparseFeatures_getNumActiveBlack.restype = ctypes.c_int

# SparseBatch
_lib.SparseBatch_getSideToMove.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getSideToMove.restype = ctypes.POINTER(ctypes.c_float)

_lib.SparseBatch_getEvaluation.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getEvaluation.restype = ctypes.POINTER(ctypes.c_float)

_lib.SparseBatch_getHalfKPFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getHalfKPFeatures.restype = ctypes.c_void_p

_lib.SparseBatch_getHalfRelativeKPFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getHalfRelativeKPFeatures.restype = ctypes.c_void_p

_lib.SparseBatch_getKingFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getKingFeatures.restype = ctypes.c_void_p

# DataLoader
_lib.DataLoader_new.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_bool]
_lib.DataLoader_new.restype = ctypes.c_void_p

_lib.DataLoader_delete.argtypes = [ctypes.c_void_p]
_lib.DataLoader_delete.restype = None

_lib.DataLoader_getBatch.argtypes = [ctypes.c_void_p]
_lib.DataLoader_getBatch.restype = ctypes.c_void_p

_lib.DataLoader_setFillVirtualFeatures.argtypes = [ctypes.c_void_p, ctypes.c_bool]
_lib.DataLoader_setFillVirtualFeatures.restype = None

_lib.DataLoader_resetEpoch.argtypes = [ctypes.c_void_p]
_lib.DataLoader_resetEpoch.restype = None


class Dataset:
    def __init__(self, file_path: str):
        self._ptr = None
        self._ptr = _lib.Dataset_new(file_path.encode('utf-8'))
        self.size = _lib.Dataset_getDataSize(self._ptr)

    def __del__(self):
        if self._ptr:
            _lib.Dataset_delete(self._ptr)
            self._ptr = None
            
class SparseFeatures:
    def __init__(self, ptr, batch_size: int, feature_size: int):
        self._ptr = ptr
        self.batch_size = batch_size
        self.feature_size = feature_size
        
        white_indices_ptr = _lib.SparseFeatures_getWhiteIndices(self._ptr)
        black_indices_ptr = _lib.SparseFeatures_getBlackIndices(self._ptr)
        
        num_active_white = _lib.SparseFeatures_getNumActiveWhite(self._ptr)
        num_active_black = _lib.SparseFeatures_getNumActiveBlack(self._ptr)
        
        white_indices = torch.transpose(torch.from_numpy(
            np.ctypeslib.as_array(white_indices_ptr, shape=(num_active_white, 2))
        ), 0, 1).long()
        black_indices = torch.transpose(torch.from_numpy(
            np.ctypeslib.as_array(black_indices_ptr, shape=(num_active_black, 2))
        ), 0, 1).long()
        
        white_features_values = torch.ones(num_active_white)
        black_features_values = torch.ones(num_active_black)
        
        self.white = torch.sparse_coo_tensor(
            white_indices, white_features_values, (self.batch_size, self.feature_size), check_invariants=False, is_coalesced=True
        )
        self.black = torch.sparse_coo_tensor(
            black_indices, black_features_values, (self.batch_size, self.feature_size), check_invariants=False, is_coalesced=True
        )

class SparseBatch:
    def __init__(self, ptr, size: int):
        self._ptr = ptr
        self.size = size
        
        side_to_move_ptr = _lib.SparseBatch_getSideToMove(self._ptr)
        evaluation_ptr = _lib.SparseBatch_getEvaluation(self._ptr)
        self.side_to_move = torch.from_numpy(
            np.ctypeslib.as_array(side_to_move_ptr, shape=(self.size, 1))
        )
        self.evaluation = torch.from_numpy(
            np.ctypeslib.as_array(evaluation_ptr, shape=(self.size, 1))
        )
        
        half_kp_ptr = _lib.SparseBatch_getHalfKPFeatures(self._ptr)
        self.half_kp = SparseFeatures(half_kp_ptr, self.size, HALF_KP_SIZE)
        
        half_relative_kp_ptr = _lib.SparseBatch_getHalfRelativeKPFeatures(self._ptr)
        self.half_relative_kp = SparseFeatures(half_relative_kp_ptr, self.size, HALF_RELATIVE_KP_SIZE)
        
        king_factor_ptr = _lib.SparseBatch_getKingFeatures(self._ptr)
        self.king_factor = SparseFeatures(king_factor_ptr, self.size, KING_FACTOR_SIZE)

class DataLoader:
    def __init__(self, dataset: Dataset, batch_size: int, num_workers: int, fill_virtual_features: bool = True):
        self._ptr = None
        self._ptr = _lib.DataLoader_new(dataset._ptr, batch_size, num_workers, fill_virtual_features)
        self._dataset = dataset
        self.batch_size = batch_size
        self.num_batches = dataset.size // self.batch_size
        self._fill_virtual_features = fill_virtual_features
        
    def reset_epoch(self):
        _lib.DataLoader_resetEpoch(self._ptr)
    
    @property
    def batch(self) -> SparseBatch:
        batch_ptr = _lib.DataLoader_getBatch(self._ptr)
        return SparseBatch(batch_ptr, self.batch_size)
    
    @property
    def fill_virtual_features(self) -> bool:
        return self._fill_virtual_features
    
    @fill_virtual_features.setter
    def fill_virtual_features(self, value: bool):
        _lib.DataLoader_setFillVirtualFeatures(self._ptr, value)
        self._fill_virtual_features = value
        
    def __del__(self):
        if self._ptr:
            _lib.DataLoader_delete(self._ptr)
            self._ptr = None
