import ctypes
import os
import platform
import numpy as np
import torch

NUM_FEATURES = 40960

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
_lib.Dataset_new.argtypes = [ctypes.c_char_p, ctypes.c_int]
_lib.Dataset_new.restype = ctypes.c_void_p

_lib.Dataset_delete.argtypes = [ctypes.c_void_p]
_lib.Dataset_delete.restype = None

# SparseBatch
_lib.SparseBatch_getSideToMove.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getSideToMove.restype = ctypes.POINTER(ctypes.c_float)

_lib.SparseBatch_getEvaluation.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getEvaluation.restype = ctypes.POINTER(ctypes.c_float)

_lib.SparseBatch_getWhiteFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getWhiteFeatures.restype = ctypes.POINTER(ctypes.c_int)

_lib.SparseBatch_getBlackFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getBlackFeatures.restype = ctypes.POINTER(ctypes.c_int)

_lib.SparseBatch_getNumActiveWhiteFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getNumActiveWhiteFeatures.restype = ctypes.c_int

_lib.SparseBatch_getNumActiveBlackFeatures.argtypes = [ctypes.c_void_p]
_lib.SparseBatch_getNumActiveBlackFeatures.restype = ctypes.c_int

# DataLoader
_lib.DataLoader_new.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
_lib.DataLoader_new.restype = ctypes.c_void_p

_lib.DataLoader_delete.argtypes = [ctypes.c_void_p]
_lib.DataLoader_delete.restype = None

_lib.DataLoader_fillBatch.argtypes = [ctypes.c_void_p]
_lib.DataLoader_fillBatch.restype = None

_lib.DataLoader_getBatch.argtypes = [ctypes.c_void_p]
_lib.DataLoader_getBatch.restype = ctypes.c_void_p

_lib.DataLoader_resetEpoch.argtypes = [ctypes.c_void_p]
_lib.DataLoader_resetEpoch.restype = None




class Dataset:
    def __init__(self, file_path: str, data_size: int):
        self._ptr = None
        self._ptr = _lib.Dataset_new(file_path.encode('utf-8'), data_size)
        self.data_size = data_size

    def __del__(self):
        if self._ptr:
            _lib.Dataset_delete(self._ptr)
            self._ptr = None

class SparseBatch:
    def __init__(self, ptr, size: int):
        self._ptr = ptr
        self.size = size
        
    @property
    def tensors(self) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor, torch.Tensor]:
        side_to_move_ptr = _lib.SparseBatch_getSideToMove(self._ptr)
        evaluation_ptr = _lib.SparseBatch_getEvaluation(self._ptr)
        white_features_ptr = _lib.SparseBatch_getWhiteFeatures(self._ptr)
        black_features_ptr = _lib.SparseBatch_getBlackFeatures(self._ptr)
        
        num_active_white_features = _lib.SparseBatch_getNumActiveWhiteFeatures(self._ptr)
        num_active_black_features = _lib.SparseBatch_getNumActiveBlackFeatures(self._ptr)
        
        
        side_to_move = torch.from_numpy(
            np.ctypeslib.as_array(side_to_move_ptr, shape=(self.size, 1))
        )
        evaluation = torch.from_numpy(
            np.ctypeslib.as_array(evaluation_ptr, shape=(self.size, 1))
        )
    
        white_features_indices = torch.transpose(torch.from_numpy(
            np.ctypeslib.as_array(white_features_ptr, shape=(num_active_white_features, 2))
        ), 0, 1).long()
        black_features_indices = torch.transpose(torch.from_numpy(
            np.ctypeslib.as_array(black_features_ptr, shape=(num_active_black_features, 2))
        ), 0, 1).long()
        
        white_features_values = torch.ones(num_active_white_features)
        black_features_values = torch.ones(num_active_black_features)
        
        white_features = torch.sparse_coo_tensor(
            white_features_indices, white_features_values, (self.size, NUM_FEATURES), check_invariants=False, is_coalesced=True

        )
        black_features = torch.sparse_coo_tensor(
            black_features_indices, black_features_values, (self.size, NUM_FEATURES), check_invariants=False, is_coalesced=True
        )
        
        return white_features, black_features, side_to_move, evaluation

class DataLoader:
    def __init__(self, dataset: Dataset, batch_size: int, num_workers: int):
        self._ptr = None
        self._ptr = _lib.DataLoader_new(dataset._ptr, batch_size, num_workers)
        self.batch_size = batch_size
        self.num_batches = dataset.data_size // self.batch_size
        
    def fill_batch(self):
        _lib.DataLoader_fillBatch(self._ptr)
        
    def reset_epoch(self):
        _lib.DataLoader_resetEpoch(self._ptr)
    
    @property
    def batch(self) -> SparseBatch:
        batch_ptr = _lib.DataLoader_getBatch(self._ptr)
        return SparseBatch(batch_ptr, self.batch_size)
        
    def __del__(self):
        if self._ptr:
            _lib.DataLoader_delete(self._ptr)
            self._ptr = None
    